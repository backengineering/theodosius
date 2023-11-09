extern "C" int MessageBoxA(void* hWnd,
                           char* lpText,
                           char* lpCaption,
                           void* uType);

struct test_t {
  char buff[0x2000];
};

test_t t = {};

void AnotherFunc();

extern "C" void EntryPoint() {
  t.buff[0] = 1;
  t.buff[1] = 2;
  AnotherFunc();
}