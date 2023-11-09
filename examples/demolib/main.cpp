#define OBF __declspec(code_seg(".obf"))

extern "C" int MessageBoxA(void* hWnd,
                           char* lpText,
                           char* lpCaption,
                           void* uType);

struct test_t {
  char* c;
  char* c2;
  char buff[0x2000];
};

test_t t = {"Hello", "World", {}};

OBF extern "C" void EntryPoint() {
  t.buff[0] = 1;
  t.buff[1] = 0;

  if (t.buff[0])
    MessageBoxA(nullptr, t.c, t.c2, nullptr);

  if (t.buff[1])
    MessageBoxA(nullptr, t.c, t.c2, nullptr);
}