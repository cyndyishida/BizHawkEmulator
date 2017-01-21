class SufamiTurbo {
public:
  struct Slot {
    MappedRAM rom;
    MappedRAM ram;
  } slotA, slotB;

  void load();
  void unload();
  void serialize(serializer&);

	SufamiTurbo();
};

extern SufamiTurbo sufamiturbo;
