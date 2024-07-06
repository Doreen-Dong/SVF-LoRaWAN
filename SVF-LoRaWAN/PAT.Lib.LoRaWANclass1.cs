using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
//the namespace must be PAT.Lib, the class and method names can be arbitrary
namespace PAT.Lib
{
    /// <summary>
    /// The math library that can be used in your model.
    /// all methods should be declared as public static.
    /// 
    /// The parameters must be of type "int", or "int array"
    /// The number of parameters can be 0 or many
    /// 
    /// The return type can be bool, int or int[] only.
    /// 
    /// The method name will be used directly in your model.
    /// e.g. call(max, 10, 2), call(dominate, 3, 2), call(amax, [1,3,5]),
    /// 
    /// Note: method names are case sensetive
    /// </summary>
    public class LoRaWANclass1
    {
	     //use interop services to import c dll. The orginal c source code can be found under Lib folder
        [DllImport(@"Lib\random.dll")]
        public static extern int RAND(int max, int min, int n);
        [DllImport(@"Lib\AES_algorithm.dll")]
        public static extern void aes128_cmac(int keyIndex, int[] msg, int len, int[] MIC);
        [DllImport(@"Lib\AES_algorithm.dll")]
        public static extern void Compute_SessionKey(int keyIndex, int AppNonce, int NetID, int DevNonce, int pad, int[] ans);
        [DllImport(@"Lib\AES_algorithm.dll")]
		public static extern void aes128_cmac2(int[] key, int[] msg, int len, int[] MIC);
		[DllImport(@"Lib\AES_algorithm.dll")]
		public static extern void aes128_encrypt(int keyIndex, int[] data, int len, int pad, int[] ans);

		
        public static bool CheckMIC(int[] mic1, int[] mic2)
        {
            bool flag = true;
            for (int i = 0; i < 4; i++)
            {
                if (mic1[i] != mic2[i])
                {
                    flag = false;
                    break;
                }
            }
            return flag;
        }
        public static bool CheckSessionKey(int[] key1, int[] key2)
		{
		    bool flag = true;
		    for (int i = 0; i<16; i++)
		    {
		        if (key1[i] != key2[i])
		        {
		            flag=false;
		            break;
		        }
		    }
		    return flag;
		}
        public static int[] Aes128_cmac(int keyIndex, int[] msg, int len)
        {
            int[] MIC = new int[4];
            aes128_cmac(keyIndex, msg, len, MIC);
            return MIC;
        }
        public static int[] Aes128_cmac2(int[] key, int[] msg, int len)
		{
		    int[] MIC = new int[4];
		    aes128_cmac2(key, msg, len, MIC);
		    return MIC;
		}
        public static int[] ComputeSessionKey(int keyIndex, int AppNonce, int NetID, int DevNonce, int pad)
        {
            int[] session = new int[16];
            Compute_SessionKey(keyIndex, AppNonce, NetID, DevNonce, pad, session);
            return session;
        }
        public static int[] Aes128_encrypt(int keyIndex, int[] data, int len, int pad)
        {
            int[] session = new int[16];
            aes128_encrypt(keyIndex, data, len, pad, session);
            return session;
        }
        
    }
}
