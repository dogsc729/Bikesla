#include "mbed.h"

#include "arm_math.h"
#include "math_helper.h"

#include "stm32l475e_iot01_accelero.h"
#include "vector"

#include <stdio.h>


#define TEST_LENGTH_SAMPLES  128//320

#define SNR_THRESHOLD_F32    75.0f
#define BLOCK_SIZE           32

#define NUM_TAPS_ARRAY_SIZE              29

#define NUM_TAPS              29


static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];

const float32_t firCoeffs32[NUM_TAPS_ARRAY_SIZE] = {
  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f
};

uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = TEST_LENGTH_SAMPLES/BLOCK_SIZE;

float32_t  snr;

static float32_t in_x[TEST_LENGTH_SAMPLES];
static float32_t in_y[TEST_LENGTH_SAMPLES];
static float32_t in_z[TEST_LENGTH_SAMPLES];
static float32_t out_x[TEST_LENGTH_SAMPLES];
static float32_t out_y[TEST_LENGTH_SAMPLES];
static float32_t out_z[TEST_LENGTH_SAMPLES];


int32_t main(void)
{
  uint32_t i;
  arm_fir_instance_f32 Sx;
  arm_fir_instance_f32 Sy;
  arm_fir_instance_f32 Sz;
  
  float32_t  *inputF32, *outputF32;
  int16_t pDataXYZ[3] = {0};

  BSP_ACCELERO_Init();

  for (i=0; i<TEST_LENGTH_SAMPLES; ++i) {
    BSP_ACCELERO_AccGetXYZ(pDataXYZ);
    in_x[i] = float(pDataXYZ[0]);
    in_y[i] = float(pDataXYZ[1]);
    in_z[i] = float(pDataXYZ[2]);
    ThisThread::sleep_for(10);
  }
  

  printf("\n\n X axis \n\n");
  printf("Raw Data:\n");
  for (i=0; i<TEST_LENGTH_SAMPLES; ++i) {
    printf(", %f", in_x[i]);
  }
  printf("\n\n");
  inputF32 = &in_x[0];
  arm_fir_init_f32(&Sx, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);

  outputF32 = &out_x[0];
  for(i=0; i < numBlocks; i++)
  {
    arm_fir_f32(&Sx, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
  }
  printf("Filtered Data\n");
  for (i=0; i<TEST_LENGTH_SAMPLES; ++i) {
    printf(", %.4f", out_x[i]);
  }
  printf("\n\n");

  printf("\n\n Y axis \n\n");
  printf("Raw Data:\n");
  for (i=0; i<TEST_LENGTH_SAMPLES; ++i) {
    printf(", %f", in_y[i]);
  }
  printf("\n\n");
  inputF32 = &in_y[0];
  arm_fir_init_f32(&Sy, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);

  outputF32 = &out_y[0];
  for(i=0; i < numBlocks; i++)
  {
    arm_fir_f32(&Sy, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
  }
  printf("Filtered Data\n");
  for (i=0; i<TEST_LENGTH_SAMPLES; ++i) {
    printf(", %.4f", out_y[i]);
  }
  printf("\n\n");

  printf("\n\n Z axis \n\n");
  printf("Raw Data:\n");
  for (i=0; i<TEST_LENGTH_SAMPLES; ++i) {
    printf(", %f", in_z[i]);
  }
  printf("\n\n");
  inputF32 = &in_z[0];
  arm_fir_init_f32(&Sz, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);

  outputF32 = &out_z[0];
  for(i=0; i < numBlocks; i++)
  {
    arm_fir_f32(&Sz, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
  }
  printf("Filtered Data\n");
  for (i=0; i<TEST_LENGTH_SAMPLES; ++i) {
    printf(", %.4f", out_z[i]);
  }
  printf("\n\n");


  while (1);
}