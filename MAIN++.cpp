#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// пины
#define TFT_CS PA4   // a4
#define TFT_RST PA3   // a3
#define TFT_DC PA2   // a2
#define TFT_MOSI PA7   // a7
#define TFT_SCLK PA5   // a5
#define TFT_LED PA1    // a1
#define TFT_MISO PA6   // a6

#define JOY_Y PB1   // b1
#define JOY_X PB0   // b0
#define BUTTON_Y PA0   // a0
#define BUTTON_X PC15  // c15
#define BUTTON_A PC14  // c14
#define BUTTON_B PC13  // c13
#define BUTTON_START PA12  // a12
#define BUTTON_SELECT PA8   // a8
#define SPEAKER_PIN PB12

// параметры дисплея
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

// цвета 
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x8430
#define ORANGE  0xFD20 // определяем оранжевый цвет

// массив цветов для фигур
const uint16_t pieceColors[] = {CYAN, BLUE, YELLOW, ORANGE, GREEN, MAGENTA, RED};
#define NUM_PIECE_COLORS (sizeof(pieceColors) / sizeof(pieceColors[0]))

// создание объектов
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// глобальные переменные
#define GRID_WIDTH 10
#define GRID_HEIGHT 14
int grid[GRID_WIDTH][GRID_HEIGHT]; // игровое поле
int currentPiece[4][2];       // текущая фигура
int currentPieceType;        // тип текущей фигуры (0-6)
int currentPieceRotation;    // поворот текущей фигуры (0-3)
int currentPieceX, currentPieceY; // позиция текущей фигуры

int nextPieceType; // тип следующей фигуры

unsigned long lastDropTime = 0;
unsigned long dropInterval = 1000; // начальная скорость падения

// размеры и формы тетриса
const int pieces[7][4][4][2] = {
  // i
  {{{0, 0}, {1, 0}, {2, 0}, {3, 0}}, {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
   {{0, 0}, {1, 0}, {2, 0}, {3, 0}}, {{1, 0}, {1, 1}, {1, 2}, {1, 3}}},
  // j
  {{{0, 0}, {0, 1}, {1, 1}, {2, 1}}, {{1, 0}, {2, 0}, {1, 1}, {1, 2}},
   {{0, 1}, {1, 1}, {2, 1}, {2, 2}}, {{1, 0}, {1, 1}, {0, 2}, {1, 2}}},
  // l
  {{{2, 0}, {0, 1}, {1, 1}, {2, 1}}, {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
   {{0, 1}, {1, 1}, {2, 1}, {0, 2}}, {{0, 0}, {1, 0}, {1, 1}, {1, 2}}},
  // o
  {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}, {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
   {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, {{0, 0}, {1, 0}, {0, 1}, {1, 1}}},
  // s
  {{{1, 0}, {2, 0}, {0, 1}, {1, 1}}, {{0, 0}, {0, 1}, {1, 1}, {1, 2}},
   {{1, 0}, {2, 0}, {0, 1}, {1, 1}}, {{0, 0}, {0, 1}, {1, 1}, {1, 2}}},
  // t
  {{{1, 0}, {0, 1}, {1, 1}, {2, 1}}, {{1, 0}, {0, 1}, {1, 1}, {1, 2}},
   {{0, 1}, {1, 1}, {2, 1}, {1, 2}}, {{1, 0}, {1, 1}, {2, 1}, {1, 2}}},
  // z
  {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}, {{1, 0}, {0, 1}, {1, 1}, {0, 2}},
   {{0, 0}, {1, 0}, {1, 1}, {2, 1}}, {{1, 0}, {0, 1}, {1, 1}, {0, 2}}}
};

// переменные для графики
#define BORDER_WIDTH 10 // ширина границы
#define GAME_OFFSET_X BORDER_WIDTH // отступ игрового поля по x
#define GAME_OFFSET_Y BORDER_WIDTH // отступ игрового поля по y
#define BLOCK_SIZE 15 // размер блока в пикселях

// координаты отображения следующей фигуры 
#define NEXT_PIECE_X (TFT_WIDTH - BORDER_WIDTH - 120)
#define NEXT_PIECE_Y 120

long score = 0;

bool paused = false;

// инициализация
void setup() {
  Serial.begin(9600);

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  pinMode(BUTTON_Y, INPUT_PULLUP);
  pinMode(BUTTON_X, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);

  tft.begin();
  tft.setRotation(3); // ориентация дисплея (0-3)
  tft.fillScreen(BLACK);
  tft.setTextSize(1);

  randomSeed(analogRead(0));
  initGame();
}

// основной цикл
void loop() {
  handleInput();

  if (paused) {
    drawPauseScreen(); // постоянно отрисовываем экран паузы
  } else {
    if (millis() - lastDropTime > dropInterval) {
      dropPiece();
      lastDropTime = millis();
    }
    drawGame(); // отрисовываем игру, если не в паузе
  }
}

//функции игры

// инициализация игрового поля и первой фигуры
void initGame() {
  memset(grid, 0, sizeof(grid)); // быстрая очистка массива
  score = 0;
  paused = false;
  currentPieceType = random(NUM_PIECE_COLORS);
  nextPieceType = random(NUM_PIECE_COLORS);  // генерируем тип следующей фигуры
  spawnNewPiece();
}

// создание новой фигуры
void spawnNewPiece() {
  currentPieceType = nextPieceType; // берем следующую фигуру
  nextPieceType = random(NUM_PIECE_COLORS); // генерируем следующую фигуру после следующей
  currentPieceRotation = 0;     // начальный поворот
  currentPieceX = GRID_WIDTH / 2 - 2;   // начальная позиция по x
  currentPieceY = 0;           // начальная позиция по y

  // копируем координаты фигуры из массива pieces
  for (int i = 0; i < 4; i++) {
    currentPiece[i][0] = pieces[currentPieceType][currentPieceRotation][i][0];
    currentPiece[i][1] = pieces[currentPieceType][currentPieceRotation][i][1];
  }

  if (!canMove(0, 0)) {
    gameOver();
  }
}

// обработка ввода с кнопок и джойстика
void handleInput() {
  if (digitalRead(BUTTON_X) == LOW) movePiece(-1, 0);
  if (digitalRead(BUTTON_Y) == LOW) movePiece(1, 0);
  if (digitalRead(BUTTON_A) == LOW) rotatePiece(1);
  if (digitalRead(BUTTON_B) == LOW) dropPiece();

  if (digitalRead(BUTTON_START) == LOW) { // пауза/возобновление игры
    paused = !paused;

    // ждем, пока кнопка не будет отпущена
    while (digitalRead(BUTTON_START) == LOW);

    return;
  }
  if (digitalRead(BUTTON_SELECT) == LOW) initGame(); // рестарт игры

  int joyX = analogRead(JOY_X);
  if (joyX < 200) movePiece(-1, 0);
  else if (joyX > 2500) movePiece(1, 0);
}

// движение фигуры
void movePiece(int dx, int dy) {
  if (canMove(dx, dy)) {
    currentPieceX += dx;
    currentPieceY += dy;
  }
}

// поворот фигуры
void rotatePiece(int direction) {
  int newRotation = (currentPieceRotation + direction) % 4;
  if (newRotation < 0) newRotation += 4;

  // временный массив для хранения повернутой фигуры
  int tempPiece[4][2];
  for (int i = 0; i < 4; i++) {
    tempPiece[i][0] = pieces[currentPieceType][newRotation][i][0];
    tempPiece[i][1] = pieces[currentPieceType][newRotation][i][1];
  }

  if (canRotate(newRotation)) {
    currentPieceRotation = newRotation;
    for (int i = 0; i < 4; i++) {
      currentPiece[i][0] = pieces[currentPieceType][currentPieceRotation][i][0];
      currentPiece[i][1] = pieces[currentPieceType][currentPieceRotation][i][1];
    }
  }
}

// проверка возможности движения фигуры
bool canMove(int dx, int dy) {
  if (paused) return false;

  for (int i = 0; i < 4; i++) {
    int newX = currentPieceX + currentPiece[i][0] + dx;
    int newY = currentPieceY + currentPiece[i][1] + dy;

    if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT || (newY >= 0 && grid[newX][newY] != 0)) {
      return false;
    }
  }
  return true;
}

// проверка возможности поворота фигуры
bool canRotate(int newRotation) {
  if (paused) return false;

  for (int i = 0; i < 4; i++) {
    int newX = currentPieceX + pieces[currentPieceType][newRotation][i][0];
    int newY = currentPieceY + pieces[currentPieceType][newRotation][i][1];

    if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT || (newY >= 0 && grid[newX][newY] != 0)) {
      return false;
    }
  }
  return true;
}

// падение фигуры вниз
void dropPiece() {
  if (canMove(0, 1)) {
    movePiece(0, 1);
  } else {
    fixPiece();
    removeFullLines();
    spawnNewPiece();
  }
}

// фиксация фигуры на игровом поле
void fixPiece() {
  for (int i = 0; i < 4; i++) {
    int x = currentPieceX + currentPiece[i][0];
    int y = currentPieceY + currentPiece[i][1];
    if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
      grid[x][y] = currentPieceType + 1; // записываем тип фигуры + 1 во избежании нуля
    }
  }
  playSound(440, 100); // воспроизводим звук при фиксации фигуры
}

// удаление заполненных линий
void removeFullLines() {
  for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[x][y] == 0) {
        fullLine = false;
        break;
      }
    }

    if (fullLine) {
      // Сдвигаем все линии *выше* текущей вниз
      for (int i = y; i > 0; i--) {
        for(int x = 0; x < GRID_WIDTH; x++){
          grid[x][i] = grid[x][i-1];
        }
      }
      // Очищаем самую верхнюю линию
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[x][0] = 0;
      }
      y++; // Проверяем эту же строку снова, так как она заполнена новой строкой
    }
  }
}

// обновление счета
void updateScore(int linesRemoved) {
  switch (linesRemoved) {
    case 1: score += 40; break;
    case 2: score += 100; break;
    case 3: score += 300; break;
    case 4: score += 1200; break;
  }
}

// конец игры
void gameOver() {
  tft.fillScreen(RED);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, 100);
  tft.println("Game Over!");
  tft.setTextSize(2);
  tft.setCursor(50, 150);
  tft.println("Press SELECT");
  
  tft.println("to restart");

  playGameOverSound(); // воспроизводим мелодию окончания игры
  while (digitalRead(BUTTON_SELECT) == HIGH); // ожидание нажатия кнопки
  initGame();
}

// графические функции

// отображение экрана паузы
void drawPauseScreen() {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, 100);
  tft.println("PAUSED");
  tft.setTextSize(2);
  tft.setCursor(50, 150);
  tft.println("Press START");
  tft.setCursor(50, 170);
  tft.println("to continue");
}

// отрисовка игры
void drawGame() {
  tft.fillScreen(BLACK);

  // рисуем границу вокруг игрового поля
  int gameAreaX1 = GAME_OFFSET_X - 2; // левая граница
  int gameAreaY1 = GAME_OFFSET_Y - 2; // верхняя граница
  int gameAreaX2 = GAME_OFFSET_X + GRID_WIDTH * BLOCK_SIZE + 2; // правая граница
  int gameAreaY2 = GAME_OFFSET_Y + GRID_HEIGHT * BLOCK_SIZE + 2; // нижняя граница
  tft.drawRect(gameAreaX1, gameAreaY1, gameAreaX2 - gameAreaX1, gameAreaY2 - gameAreaY1, WHITE);

  // отрисовка игрового поля
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      if (grid[x][y] != 0) {
        drawBlock(x, y, getColor(grid[x][y] - 1)); // -1 т.к. в grid хранится тип + 1
      }
    }
  }

  // отрисовка текущей фигуры
  for (int i = 0; i < 4; i++) {
    int x = currentPieceX + currentPiece[i][0];
    int y = currentPieceY + currentPiece[i][1];
    if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
      drawBlock(x, y, getColor(currentPieceType));
    }
  }

  // отрисовка следующей фигуры
  drawNextPiece(nextPieceType);

  // вывод счета
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(TFT_WIDTH - BORDER_WIDTH - 120, 30); // позиция для счета
  tft.print("Score: ");
  tft.println(score);
}

// отрисовка одного блока
void drawBlock(int x, int y, uint16_t color) {
  int pixelX = GAME_OFFSET_X + x * BLOCK_SIZE;
  int pixelY = GAME_OFFSET_Y + y * BLOCK_SIZE;
  tft.fillRect(pixelX, pixelY, BLOCK_SIZE, BLOCK_SIZE, color);
  tft.drawRect(pixelX, pixelY, BLOCK_SIZE, BLOCK_SIZE, GRAY);
}

// получение цвета для фигуры
uint16_t getColor(int pieceType) {
  return pieceColors[pieceType % NUM_PIECE_COLORS];
}

// функция для отрисовки следующей фигуры
void drawNextPiece(int pieceType) {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(NEXT_PIECE_X, NEXT_PIECE_Y - 20);
  tft.println("Next:");

  // отрисовываем следующую фигуру
  for (int i = 0; i < 4; i++) {
    int x = pieces[pieceType][0][i][0];
    int y = pieces[pieceType][0][i][1];
    int pixelX = NEXT_PIECE_X + x * BLOCK_SIZE; // выравнивание
    int pixelY = NEXT_PIECE_Y + y * BLOCK_SIZE;

    tft.fillRect(pixelX, pixelY, BLOCK_SIZE, BLOCK_SIZE, getColor(pieceType));
    tft.drawRect(pixelX, pixelY, BLOCK_SIZE, BLOCK_SIZE, GRAY);
  }
}

// функция для воспроизведения звука
void playSound(int frequency, int duration) {
  tone(SPEAKER_PIN, frequency, duration);
  delay(duration);
  noTone(SPEAKER_PIN);
}

// функция для воспроизведения звука окончания игры
void playGameOverSound() {
  int melody[] = {
    570, 659, 0, 0, 0, 659, 0, 520,
    0, 479, 0, 419, 0, 378, 0, 359,
    0, 0, 350, 0, 350
  };
  int durations[] = {
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5
  };

  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    tone(SPEAKER_PIN, melody[i], 500/durations[i]); 
    delay(500 / durations[i] * 1.30);
    noTone(SPEAKER_PIN);
    delay(10);
  }
}