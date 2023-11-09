#define OBF __declspec(code_seg(".obf"))

extern "C" int MessageBoxA(void* hWnd,
                           char* lpText,
                           char* lpCaption,
                           void* uType);

struct test_t {
  char buff[0x2000];
};

extern test_t t;

OBF void AnotherFunc() {
  if (t.buff[0])
    MessageBoxA(nullptr, "Hello World", "Hello World", nullptr);

  if (t.buff[1])
    MessageBoxA(nullptr, "Hello World 1", "Hello World 1", nullptr);
}