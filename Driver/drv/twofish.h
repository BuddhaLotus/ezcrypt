//modified by Stefan Scherrer to allow compilation under DDK
typedef struct fish2_key
{ DWORD32 keyLen;       /* Key Length in Bit */
  DWORD32 sboxKeys[4];
  DWORD32 subKeys[40];
  UCHAR  key[32];
  DWORD32 sbox_full[1024];  /* This have to be 1024 DWORDs */
} fish2_key;

void init_key(fish2_key * key);

int blockDecrypt_CBC(fish2_key *key,UCHAR *src,UCHAR *dst,DWORD32 len);
int blockEncrypt_CBC(fish2_key *key,UCHAR *src,UCHAR *dst,DWORD32 len);
