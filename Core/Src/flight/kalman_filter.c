/*
 * kf.c
 *
 *  Created on: 12 Apr 2022
 *      Author: Luca
 */

#define STD_NOISE_ACC 5.0f // m/s^2
#define STD_NOISE_ALT 5.0f // meter
#define SAMPLE_TIME 0.01f  // seconds

#include "kalman_filter.h"
#include "util/linalg.h"
#include <string.h>

void kalman_init(kalman_filter_t *filter) {

  /* Initialize to default values */
  filter->ts = SAMPLE_TIME;
  filter->R = STD_NOISE_ALT;

  float Ad[] = {1, filter->ts, 0, 1};
  memcpy(filter->Ad, Ad, sizeof(Ad));
  tran(Ad, 2, 2); // Transform matrix
  memcpy(filter->Ad_T, Ad, sizeof(Ad));

  float Gd[2 * 1] = {(filter->ts * filter->ts) / 2, filter->ts};
  float QGd_T[1 * 2] = {((filter->ts * filter->ts) / 2) * STD_NOISE_ACC,
                        filter->ts * STD_NOISE_ACC};
  mul(Gd, QGd_T, filter->GdQGd_T, 2, 1, 2);

  float x_hat[] = {0, 0};
  memcpy(filter->x_hat, x_hat, sizeof(x_hat));

  float x_bar[] = {0, 0};
  memcpy(filter->x_bar, x_bar, sizeof(x_bar));

  float H[] = {1, 0};
  memcpy(filter->H, H, sizeof(H));
  memcpy(filter->H_T, H, sizeof(H));

  float K[] = {0, 0};
  memcpy(filter->K, K, sizeof(K));

  float P_hat[] = {1, 0, 0, 1};
  memcpy(filter->P_hat, P_hat, sizeof(P_hat));

  float P_bar[] = {1, 0, 0, 1};
  memcpy(filter->P_bar, P_bar, sizeof(P_bar));
}

void kalman_reset() {}

void kalman_predict(kalman_filter_t *filter) {
  /* Project the state ahead */
  /* x_hat = Ad*x_bar + Bd*u */
  mul(filter->Ad, filter->x_bar, filter->x_hat, 2, 2, 1);

  /* Project the error covariance ahead */
  /* P_hat = Ad*P_bar*Ad_T + Gd*Q*Gd_T */
  float P_barAd_T[4];
  mul(filter->P_bar, filter->Ad_T, P_barAd_T, 2, 2, 2);
  mul(filter->Ad, P_barAd_T, filter->P_hat, 2, 2, 2);
  add(filter->P_hat, filter->GdQGd_T, filter->P_hat, 4);
}

void kalman_update(kalman_filter_t *filter) {
  /* Compute Kalman Gain */
  /* K = P_hat*H_T*(H*P_hat*H_T + R)^-1 */
  float P_hatH_T[2];
  mul(filter->P_hat, filter->H_T, P_hatH_T, 2, 2, 1);
  float HP_hatH_T;
  mul(filter->H, P_hatH_T, &HP_hatH_T, 1, 2, 1);
  HP_hatH_T = 1 / (HP_hatH_T + filter->R);
  mul(P_hatH_T, &HP_hatH_T, filter->K, 2, 1, 1);

  /* Update the estimate */
  /* x_bar = x_hat+K*(z-H*x_hat); */
  float Hx_hat;
  mul(filter->H, filter->x_hat, &Hx_hat, 1, 2, 1);
  Hx_hat = filter->altitude_agl - Hx_hat;
  float KZ[2];
  mul(filter->K, &Hx_hat, KZ, 2, 1, 1);
  add(filter->x_hat, KZ, filter->x_bar, 2);

  /* Update the error covariance */
  /* P_bar = (eye-K*H)*P_hat */
  float KH[4];
  mul(filter->K, filter->H, KH, 2, 1, 2);
  float eye[4] = {1.0f, 0, 0, 1.0f};
  sub(eye, KH, KH, 4);
  mul(KH, filter->P_hat, filter->P_bar, 2, 2, 2);
}
