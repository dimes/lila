CC := cc
SRC := src
BUILD := build

SOURCES := $(wildcard $(SRC)/*.c)
HEADERS := $(wildcard $(SRC)/*.h)
OBJECTS := $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SOURCES))

all: setup lila

setup:
	mkdir -p $(BUILD)

clean:
	rm -r $(BUILD)

lila: $(OBJECTS) $(HEADERS)
	$(CC) $(OBJECTS) -o "$(BUILD)/$@"

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) -I$(SRC) -c $< -o $@
