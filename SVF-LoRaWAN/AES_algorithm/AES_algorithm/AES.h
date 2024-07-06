
#define DllExport   __declspec( dllexport )
extern "C" DllExport void aes128_cmac(int keyIndex, int* msg, int len,int* MIC);
extern "C" DllExport void Compute_SessionKey(int keyIndex, int AppNonce, int NetID, int DevNonce, int pad,int* ans);
extern "C" DllExport void aes128_cmac2(int* key, int* msg, int len,int* MIC);
extern "C" DllExport void aes128_encrypt(int keyIndex, int* data, int len, int pad, int* ans);

