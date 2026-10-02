/*This sketch is a simple version of the famous Simon Says game. You can  use it and improved it adding
levels and everything you want to increase the  diffuculty!

There are five buttons connected to A0, A1, A2, A3 and A4.
The  buttons from A0 to A3 are used to insert the right sequence while A4 to start the  game.

When a wrong sequence is inserted all the leds will blink for three  time very fast otherwhise the
inserted sequence is correct.

Hardware needed:
5x  pushbuttons
1x Blue led
1x Yellow led
1x Red led
1x Green Led
4x  1k resistors
4x 10k resisors
10x jumpers
*/

const int MAX_LEVEL = 100;
const int BUTTON_COUNT = 4;

int sequence[MAX_LEVEL];
int level = 1;
int velocity = 800;

const int ledpin1 = 14;
const int ledpin2 = 32;
const int ledpin3 = 15;
const int ledpin4 = 33;

const int buttonPins[BUTTON_COUNT] = {A0, A1, A2, A3};

const int ledPins[BUTTON_COUNT] = {ledpin1, ledpin4, ledpin3, ledpin2};
const int startButton = A1;

void waitForStart();
void show_sequence();
bool get_sequence();
int waitForButton();
void generate_sequence();
void wrong_sequence();
void right_sequence();
void winning_sequence();
void allLedsOn();
void allLedsOff();

void setup() {
	for (int i = 0; i < BUTTON_COUNT; i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);
		pinMode(ledPins[i], OUTPUT);
		digitalWrite(ledPins[i], LOW);
	}

	pinMode(startButton, INPUT_PULLUP);
	randomSeed(micros());
}

void loop() {
	waitForStart();
	generate_sequence();

	while (level <= MAX_LEVEL) {
		show_sequence();

		if (!get_sequence()) {
			wrong_sequence();
			return;
		}

		right_sequence();

		if (level == MAX_LEVEL) {
			winning_sequence();
			return;
		}

		level++;
		if (velocity > 250) {
			velocity -= 50;
		}
	}
}

void waitForStart() {
	allLedsOff();

	while (digitalRead(startButton) == HIGH) {
		delay(10);
	}

	delay(50);
	while (digitalRead(startButton) == LOW) {
		delay(10);
	}
}

void show_sequence() {
	allLedsOff();
	delay(400);

	for (int i = 0; i < level; i++) {
		digitalWrite(ledPins[sequence[i]], HIGH);
		delay(velocity);
		digitalWrite(ledPins[sequence[i]], LOW);
		delay(200);
	}
}

bool get_sequence() {
	for (int i = 0; i < level; i++) {
		int pressedButton = waitForButton();

		if (pressedButton != sequence[i]) {
			return false;
		}
	}

	return true;
}

int waitForButton() {
	while (true) {
		for (int i = 0; i < BUTTON_COUNT; i++) {
			if (digitalRead(buttonPins[i]) == LOW) {
				digitalWrite(ledPins[i], HIGH);
				delay(50);

				while (digitalRead(buttonPins[i]) == LOW) {
					delay(10);
				}

				digitalWrite(ledPins[i], LOW);
				delay(50);
				return i;
			}
		}
		delay(5);
	}
}

void generate_sequence() {
	for (int i = 0; i < MAX_LEVEL; i++) {
		sequence[i] = random(BUTTON_COUNT);
	}
}

void wrong_sequence() {
	for (int i = 0; i < 3; i++) {
		allLedsOn();
		delay(250);
		allLedsOff();
		delay(250);
	}

	level = 1;
	velocity = 800;
}

void right_sequence() {
	allLedsOff();
	delay(250);
	allLedsOn();
	delay(400);
	allLedsOff();
	delay(300);
}

void winning_sequence() {
	for (int i = 0; i < 5; i++) {
		allLedsOn();
		delay(150);
		allLedsOff();
		delay(150);
	}

	level = 1;
	velocity = 800;
}

void allLedsOn() {
	for (int i = 0; i < BUTTON_COUNT; i++) {
		digitalWrite(ledPins[i], HIGH);
	}
}

void allLedsOff() {
	for (int i = 0; i < BUTTON_COUNT; i++) {
		digitalWrite(ledPins[i], LOW);
	}
}