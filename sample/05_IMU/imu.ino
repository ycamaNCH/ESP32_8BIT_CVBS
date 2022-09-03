#include <Arduino.h>

#include <M5Atom.h>
#include <M5GFX.h>
#include <ESP32_8BIT_CVBS.h>
static ESP32_8BIT_CVBS display;
static M5Canvas        canvas(&display);

typedef struct {
  double x;
  double y;
  double z;
} point_3d_t;

typedef struct {
  point_3d_t start_point;
  point_3d_t end_point;
} line_3d_t;

typedef struct {
  double x;
  double y;
} point_2d_t;

double r_rand = PI / 180;

double r_alpha = 19.47 * PI / 180;
double r_gamma = 20.7 * PI / 180;

double sin_alpha = sin(19.47 * PI / 180);
double cos_alpha = cos(19.47 * PI / 180);
double sin_gamma = sin(20.7 * PI / 180);
double cos_gamma = cos(20.7 * PI / 180);

void RotatePoint(point_3d_t *point, double x, double y, double z) {
  if (x != 0) {
    point->y = point->y * cos(x * r_rand) - point->z * sin(x * r_rand);
    point->z = point->y * sin(x * r_rand) + point->z * cos(x * r_rand);
  }

  if (y != 0) {
    point->x = point->z * sin(y * r_rand) + point->x * cos(y * r_rand);
    point->z = point->z * cos(y * r_rand) - point->x * sin(y * r_rand);
  }

  if (z != 0) {
    point->x = point->x * cos(z * r_rand) - point->y * sin(z * r_rand);
    point->y = point->x * sin(z * r_rand) + point->y * cos(z * r_rand);
  }
}

void RotatePoint(point_3d_t *point, point_3d_t *point_new, double x, double y, double z) {
  if (x != 0) {
    point_new->y = point->y * cos(x * r_rand) - point->z * sin(x * r_rand);
    point_new->z = point->y * sin(x * r_rand) + point->z * cos(x * r_rand);
  }

  if (y != 0) {
    point_new->x = point->z * sin(y * r_rand) + point->x * cos(y * r_rand);
    point_new->z = point->z * cos(y * r_rand) - point->x * sin(y * r_rand);
  }

  if (z != 0) {
    point_new->x = point->x * cos(z * r_rand) - point->y * sin(z * r_rand);
    point_new->y = point->x * sin(z * r_rand) + point->y * cos(z * r_rand);
  }
}

line_3d_t rect[12] = {
    {.start_point = {-1, -1, 1}, .end_point = {1, -1, 1}},
    {.start_point = {1, -1, 1}, .end_point = {1, 1, 1}},
    {.start_point = {1, 1, 1}, .end_point = {-1, 1, 1}},
    {.start_point = {-1, 1, 1}, .end_point = {-1, -1, 1}},
    {
        .start_point = {-1, -1, 1},
        .end_point   = {-1, -1, -1},
    },
    {
        .start_point = {1, -1, 1},
        .end_point   = {1, -1, -1},
    },
    {
        .start_point = {1, 1, 1},
        .end_point   = {1, 1, -1},
    },
    {
        .start_point = {-1, 1, 1},
        .end_point   = {-1, 1, -1},
    },
    {.start_point = {-1, -1, -1}, .end_point = {1, -1, -1}},
    {.start_point = {1, -1, -1}, .end_point = {1, 1, -1}},
    {.start_point = {1, 1, -1}, .end_point = {-1, 1, -1}},
    {.start_point = {-1, 1, -1}, .end_point = {-1, -1, -1}},
};

bool point3Dto2D(point_3d_t *source, point_2d_t *point) {
  point->x = (source->x * cos_gamma) - (source->y * sin_gamma);
  point->y = -(source->x * sin_gamma * sin_alpha) - (source->y * cos_gamma * sin_alpha) + (source->z * cos_alpha);
  return true;
}

bool point2DToDisPoint(point_2d_t *point, uint8_t *x, uint8_t *y) {
  *x = point->x + 120;
  *y = 67 - point->y;
  return true;
}

bool printLine3D(M5Canvas *canvas, line_3d_t *line, uint16_t color) {
  uint8_t    start_x, start_y, end_x, end_y;
  point_2d_t point;
  point3Dto2D(&line->start_point, &point);
  point2DToDisPoint(&point, &start_x, &start_y);
  point3Dto2D(&line->end_point, &point);
  point2DToDisPoint(&point, &end_x, &end_y);

  canvas->drawLine(start_x, start_y, end_x, end_y, color);

  return true;
}

void MPU6886Test() {
  float accX = 0;
  float accY = 0;
  float accZ = 0;

  double theta = 0, last_theta = 0;
  double phi = 0, last_phi = 0;
  double alpha = 0.2;

  line_3d_t x = {.start_point = {0, 0, 0}, .end_point = {0, 0, 0}};
  line_3d_t y = {.start_point = {0, 0, 0}, .end_point = {0, 0, 0}};
  line_3d_t z = {.start_point = {0, 0, 0}, .end_point = {0, 0, 30}};

  line_3d_t rect_source[12];
  line_3d_t rect_dis;
  for (int n = 0; n < 12; n++) {
    rect_source[n].start_point.x = rect[n].start_point.x * 30;
    rect_source[n].start_point.y = rect[n].start_point.y * 30;
    rect_source[n].start_point.z = rect[n].start_point.z * 30;
    rect_source[n].end_point.x   = rect[n].end_point.x * 30;
    rect_source[n].end_point.y   = rect[n].end_point.y * 30;
    rect_source[n].end_point.z   = rect[n].end_point.z * 30;
  }

  while (1) {
    M5.IMU.getAccelData(&accX, &accY, &accZ);

    if ((accX < 1) && (accX > -1)) {
      theta = asin(-accX) * 57.295;
    }
    if (accZ != 0) {
      phi = atan(accY / accZ) * 57.295;
    }

    theta = alpha * theta + (1 - alpha) * last_theta;
    phi   = alpha * phi + (1 - alpha) * last_phi;

    canvas.fillSprite(TFT_BLACK);
    canvas.setTextSize(1);
    canvas.setCursor(10, 115);
    canvas.printf("%.2f", theta);
    canvas.setCursor(10, 125);
    canvas.printf("%.2f", phi);

    // delay(20);

    z.end_point.x = 0;
    z.end_point.y = 0;
    z.end_point.z = 60;
    RotatePoint(&z.end_point, theta, phi, 0);
    RotatePoint(&z.end_point, &x.end_point, -90, 0, 0);
    RotatePoint(&z.end_point, &y.end_point, 0, 90, 0);

    for (int n = 0; n < 12; n++) {
      RotatePoint(&rect_source[n].start_point, &rect_dis.start_point, theta, phi, (double)0);
      RotatePoint(&rect_source[n].end_point, &rect_dis.end_point, theta, phi, (double)0);
      printLine3D(&canvas, &rect_dis, TFT_WHITE);
    }
    // canvas.fillRect(0,0,160,80,BLACK);
    printLine3D(&canvas, &x, TFT_RED);
    printLine3D(&canvas, &y, TFT_GREEN);
    printLine3D(&canvas, &z, TFT_BLUE);

    /*
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(1);
    canvas.fillRect(0,0,52,18,canvas.color565(20,20,20));
    canvas.drawString("MPU6886",5,5,1);
    */

    last_theta = theta;
    last_phi   = phi;

    // M5.update();
    //  checkAXPPress();
    // canvas.pushSprite(0, 0);
    canvas.pushRotateZoom(0, 1.1, 1.2);
    display.display();
  }
}

void setup() {
  display.begin();
  display.setColorDepth(8);
  display.setRotation(0);
  display.fillScreen(TFT_BLACK);
  display.setPivot((240 >> 1) + 5, (200 >> 1) + 45);
  display.startWrite();

  if (!canvas.createSprite(240, 200)) {
    log_e("cannot allocate sprite buffer");
  }

  canvas.setColorDepth(16);

  M5.begin(false, true, false);
  M5.IMU.Init();  // Init IMU sensor.
}

void loop() {
  MPU6886Test();
}
