/**
 * simple extended Needham-Schroeder implementation
 *
 * Copyright (c) 2013 Andreas Bender <bender@tzi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "rin_wrapper.h"
#include "ns_util.h"

#include <string.h>
#include <stdio.h>

#include <openssl/ssl.h>

/*
 * Taken from the tinydtls-Lib. ( MIT License, http://tinydtls.sourceforge.net/ )
 */
void 
ns_dump(unsigned char *buf, size_t len) {
  printf("\t");
  size_t i = 0;
  while (i < len) {
    printf("%02x ", buf[i++]);
    if (i % 4 == 0)
      printf(" ");
    if (i % 16 == 0)
      printf("\n\t");
  }
  printf("\n");
}

extern int g_encryption;

int ns_encrypt(u_char *key, u_char *src, u_char *dst, size_t length, size_t key_length) {
    
    if(key_length != 16) {
        ns_log_warning(" key_length != 16 which will likely lead to an Bus error or "
                       "segmentation fault!");
    }
    
    if(length % NS_BLOCK_SIZE != 0) {
        ns_log_warning("The length of the src must be a multiple of NS_BLOCK_SIZE "
                       "(which is %d) but is %d.", NS_BLOCK_SIZE, length);
    }
    
    if (g_encryption == NS_ENCRYPTION_RIJNDAEL)
    {

        
        rijndael_ctx ctx;
        rijndael_set_key(&ctx, key, 8 * key_length);
        
        int j;
        
        // Encrypt blockwise from src to dst.
        for(j = 0; j < length; j += NS_BLOCK_SIZE) { 
            rijndael_encrypt(&ctx, &(src[j]), &(dst[j])); 
        }
        
        return 0;
        
    } else if(g_encryption == NS_ENCRYPTION_BLOWFISH)
    {
        //printf("encrypt using blowfish\n");
        EVP_CIPHER_CTX cipherHandle;
        EVP_CIPHER_CTX_init(&cipherHandle);
        char iv[] = "01234567";

        EVP_EncryptInit(&cipherHandle, EVP_bf_cbc(), (const unsigned char*)key,
                        (const unsigned char*)iv);
        EVP_CIPHER_CTX_set_padding(&cipherHandle, 0);

        memset(dst, 0, length);
        int outlen = 0, tmplen;
        if (!EVP_EncryptUpdate(&cipherHandle, (unsigned char *)dst, &outlen,
                              (const unsigned char *)src, length))
        {
            printf("error encrypting data, blowfish\n");
            return -1;
        }
        if (!EVP_EncryptFinal(&cipherHandle, (unsigned char*) (dst + outlen),
                              &tmplen))
        {
            printf("error encrypting data, blowfish\n");
            return -1;
        }
        outlen += tmplen;
        
        if(outlen != length)
        {
            printf("blowfish encryption: out_len != length, out_len=%d, length=%d\n",
                   outlen, length);
        }
        EVP_CIPHER_CTX_cleanup(&cipherHandle);
        return 0;
    }
    else
    {
        printf("ns_encrypt failed, unsupported method\n");
        return -1;
    }
    return 0;
}

int ns_encrypt_pkcs7(u_char *key, u_char *src, u_char *dst, size_t length, size_t key_length) {
  
  if(length % NS_BLOCK_SIZE != 0) { // padding required.
    size_t padded_length = length + NS_BLOCK_SIZE - (length % NS_BLOCK_SIZE);
    u_char padded_src[padded_length];
    memcpy(padded_src, src, length);
    memset(&padded_src[length], (padded_length - length), padded_length - length);
    return ns_encrypt(key, padded_src, dst, padded_length, key_length);
    
  } else { // no padding required, normal encryption
    return ns_encrypt(key, src, dst, length, key_length);
  }
  
}

int ns_decrypt(u_char *key, u_char *src, u_char *dst, size_t length, size_t key_length) {
    if(key_length != 16) {
        ns_log_warning(" key_length != 16 which will likely lead to an Bus error or "
                       "segmentation fault!");
    }
    
    if(length % NS_BLOCK_SIZE != 0) {
        ns_log_warning("The length of the src must be a multiple of NS_BLOCK_SIZE "
                       "(which is %d) but is %d.", NS_BLOCK_SIZE, length);
    }
    
  if (g_encryption == NS_ENCRYPTION_RIJNDAEL)
  {

      
      rijndael_ctx ctx;
      rijndael_set_key(&ctx, key, 8 * key_length);
      
      int j;
      
      // Decrypt blockwise from src to dst.
      for(j = 0; j < length; j += NS_BLOCK_SIZE) {
          rijndael_decrypt(&ctx, &(src[j]), &(dst[j])); 
      }
      
      return 0;
  } else if(g_encryption == NS_ENCRYPTION_BLOWFISH)
  {
      //printf("decrypt using blowfish\n");

      EVP_CIPHER_CTX cipherHandle;
      EVP_CIPHER_CTX_init(&cipherHandle);
      char iv[] = "01234567";
      EVP_DecryptInit(&cipherHandle, EVP_bf_cbc(), (const unsigned char*)key,
                      (const unsigned char*)iv);
      EVP_CIPHER_CTX_set_padding(&cipherHandle, 0);


      memset(dst, 0, length);
      int outlen = 0, tmplen;
      if (!EVP_DecryptUpdate(&cipherHandle, (unsigned char *)dst, &outlen,
                             (const unsigned char *)src, length))
      {
          printf("error decrypting data update, blowfish\n");
          return -1;
      }
      if (!EVP_DecryptFinal(&cipherHandle, (unsigned char*) (dst + outlen),
                            &tmplen))
      {
          printf("error decrypting data final, blowfish\n");
          return -1;
      }
      outlen += tmplen;
      
      if(outlen != length)
      {
          printf("blowfish encryption: out_len != length, out_len=%d, length=%d\n",
                 outlen, length);
      }
      EVP_CIPHER_CTX_cleanup(&cipherHandle);
      return 0;
  } else
    {
        printf("ns_decrypt failed, unsupported method\n");
        return -1;
    }
    return 0;
      
}

/**
 * Encrypts and decrypts a text.
 */
int ns_test_encryption() {
  
  int BUFF_SIZE = 40;
                 
  u_char C_KEY[] = { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
                     0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF};

  u_char C_TEXT[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                      0x18, 0x19, 0x1A, 0x1B, 0x00, 0x00, 0x00, 0x00};
                   
  u_char buf1[BUFF_SIZE];
  u_char buf2[BUFF_SIZE];
  
  memset(buf1, 0, sizeof(buf1));
  memset(buf2, 0, sizeof(buf2));
  
  ns_encrypt(C_KEY, C_TEXT, buf1, sizeof(C_TEXT), sizeof(C_KEY));
  
  ns_dump(buf1, BUFF_SIZE);

  ns_decrypt(C_KEY, buf1, buf2, sizeof(C_TEXT), sizeof(C_KEY));
  
  ns_dump(buf2, BUFF_SIZE);
  
  return 0;
}