////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

AES AESGen;
uint8_t WZKeys[3][0x10000];

void GenWzKey(const uint8_t* IV, uint8_t* key) {
	uint8_t BigIV[16];
	for (int i = 0; i < 16; i += 4) {
		memcpy(BigIV+i, IV, 4);
	}
	AESGen.SetParameters(256, 128);
	AESGen.StartEncryption(AESKey2);
	AESGen.EncryptBlock(BigIV, key);
	for (int i = 16; i < 0x10000; i += 16) {
		AESGen.EncryptBlock(key+i-16, key+i);
	}
}

void Crypto::TransformData(uint8_t* IV, uint8_t* data, uint32_t len) {
	uint8_t BigIV[16];
	for (int i = 0; i < 16; i += 4) {
		memcpy(BigIV+i, IV, 4);
	}

	uint32_t pos = 0;
	uint8_t first = 1;
	int32_t tpos = 0;
	while (len > pos) {
		AESGen.SetParameters(256, 128);
		AESGen.StartEncryption(AESKey2);
		tpos = 1460 - first * 4;
		if (len > pos + tpos) {
			AESGen.TransformOFB(data + pos, BigIV, tpos);
		}
		else {
			AESGen.TransformOFB(data + pos, BigIV, len - pos);
		}
		pos += tpos;
		if (first) first = 0;
	}
}

void Crypto::Init() {
	GenWzKey(GMSKeyIV, WZKeys[1]);
	GenWzKey(KMSKeyIV, WZKeys[2]);
}