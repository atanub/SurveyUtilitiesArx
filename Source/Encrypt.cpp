//Encrypt.cpp
/*
Encryption Example
This example reads data from a text file (test1.txt), encrypts it using the RC2 block cipher, and writes out the encrypted data to another file (test1.xxx). A random session key is generated to perform the encryption and is stored to the output file along with the encrypted data. Note that this session key is encrypted with our own public key exchange key by the CryptExportKey function. 
*/
#include "stdafx.h"
#include <wincrypt.h>

void EncryptFile()
{
	FILE *hSource = NULL;
	FILE *hDest = NULL;
	int eof = 0;

	HCRYPTPROV hProv = 0;
	HCRYPTKEY hKey = 0;
	HCRYPTKEY hXchgKey = 0;

	#define BLOCK_SIZE 160
	#define BUFFER_SIZE (BLOCK_SIZE+16) // Give buffer 16 bytes of extra
										// room for padding, etc.
	BYTE pbBuffer[BUFFER_SIZE];
	DWORD dwCount;

	BYTE *pbKeyBlob = NULL;
	DWORD dwBlobLen;

	// Open source file.
	if((hSource=fopen("test1.txt","rb"))==NULL) {
		printf("Error opening source file!\n");
		goto done;
	}

	// Open destination file.
	if((hDest=fopen("test1.xxx","wb"))==NULL) {
		printf("Error opening destination file!\n");
		goto done;
	}

	// Get handle to the default provider.
	if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0)) {
		printf("Error %x during CryptAcquireContext!\n", GetLastError());
		goto done;
	}

	// Get handle to key exchange key.
	if(!CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hXchgKey)) {
		printf("Error %x during CryptGetUserKey!\n", GetLastError());
		goto done;
	}

	// Create a random block cipher session key.
	if(!CryptGenKey(hProv, CALG_RC2, CRYPT_EXPORTABLE, &hKey)) {
		printf("Error %x during CryptGenKey!\n", GetLastError());
		goto done;
	}

	// Determine size of key blob and allocate memory.
	if(!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, NULL, &dwBlobLen)) {
		printf("Error %x computing blob length!\n", GetLastError());
		goto done;
	}
	if((pbKeyBlob = malloc(dwBlobLen)) == NULL) {
		printf("Out of memory!\n");
		goto done;
	}

	// Export key into a simple key blob.
	if(!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, pbKeyBlob, &dwBlobLen)) {
		printf("Error %x during CryptExportKey!\n", GetLastError());
		free(pbKeyBlob);
		goto done;
	}

	// Write size of key blob to destination file.
	fwrite(&dwBlobLen, sizeof(DWORD), 1, hDest);
	if(ferror(hDest)) {
		printf("Error writing header!\n");
		free(pbKeyBlob);
		goto done;
	}

	// Write key blob to destination file.
	fwrite(pbKeyBlob, 1, dwBlobLen, hDest);
	if(ferror(hDest)) {
		printf("Error writing header!\n");
		free(pbKeyBlob);
		goto done;
	}

	// Free memory.
	free(pbKeyBlob);

	// Encrypt source file and write to destination file.
	do {
		// Read up to BLOCK_SIZE bytes from source file.
		dwCount = fread(pbBuffer, 1, BLOCK_SIZE, hSource);
		if(ferror(hSource)) {
			printf("Error reading data!\n");
			goto done;
		}
		eof=feof(hSource);

		// Encrypt data
		if(!CryptEncrypt(hKey, 0, eof, 0, pbBuffer, &dwCount, BUFFER_SIZE)) {
			printf("Error %x during CryptEncrypt!\n", GetLastError());
			goto done;
		}

		// Write data to destination file.
		fwrite(pbBuffer, 1, dwCount, hDest);
		if(ferror(hDest)) {
			printf("Error writing data!\n");
			goto done;
		}
	} while(!feof(hSource));

	done:

	// Destroy session key.
	if(hKey != 0) CryptDestroyKey(hKey);

	// Destroy key exchange key.
	if(hXchgKey != 0) CryptDestroyKey(hXchgKey);

	// Release provider handle.
	if(hProv != 0) CryptReleaseContext(hProv, 0);

	// Close source file.
	if(hSource != NULL) fclose(hSource);

	// Close destination file.
	if(hDest != NULL) fclose(hDest);
}
