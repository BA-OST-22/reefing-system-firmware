#pragma once

typedef struct {
  float altitude_agl;
  float Ad[2*2];
  float Ad_T[2*2];
  float GdQGd_T[2*2];
  float H[1*2];
  float H_T[2*1];
  float K[2*1];
  float x_bar[2*1];
  float x_hat[2*1];
  float P_bar[2*2];
  float P_hat[2*2];
  float R;
  float ts;
} kalman_filter_t;

void kalman_init(kalman_filter_t* filter);
void kalman_predict(kalman_filter_t* filter);
void kalman_update(kalman_filter_t* filter);
