//Author: Tomasz Juszczak; Author of basic code (under) Jakub Trznadel http://trznadel.info/kuba/ 

#include "stdafx.h"
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include <Windows.h>

using namespace std;

#define MAX_PRIME (1ll<<32)
#define SQRT_MAX_PRIME (1<<16)
#define MAX_TASK (6)
#define MAX_S (20)

bool save = false;
mutex mI;
uint32_t index = 1;
uint32_t arPrime[MAX_PRIME / 32 / 2];
uint32_t z[64];
uint32_t y[MAX_TASK][MAX_S][32];
uint32_t prime[MAX_PRIME / 32 / 2];
uint32_t sum = 1;
inline void clrpixel(uint64_t i) { arPrime[i >> 6] &= ~(z[i & 0x3F]); }
inline uint32_t getpixel(uint32_t i) { return (arPrime[i >> 6] & z[i & 0x3F]); }
inline uint32_t  getpixelprime(uint32_t i) { return (prime[i >> 6] & z[i & 0x3F]); }
inline void clrpixelprime(uint64_t i) { prime[i >> 6] &= ~(z[i & 0x3F]); }

inline void fillz() {
	z[64] = 0;
	for (int i = 0; i < 31; i++) {
		z[i * 2 + 1] = 1 << i;
		z[i * 2] = 0;
	}
}

inline void copyPrime() {
	for (int i = 0; i < MAX_PRIME / 32 / 2; i++) {
		prime[i] = arPrime[i];
	}
}
void fill()
{
	fillz();
	uint64_t i, j;
	for (i = 0; i < MAX_PRIME / 32 / 2; i++)
		arPrime[i] = ~0;
	cout << "filled table" << endl;

	for (i = 3; i < SQRT_MAX_PRIME; i += 2)
	{
		if (!getpixel(i)) continue;
		for (j = 3 * i; j < MAX_PRIME; j += 2 * i)
			clrpixel(j);
	}
	cout << "clear all not first" << endl;
	copyPrime();
}

inline void fillPowerModulo(uint8_t task, uint8_t id, uint32_t no)
{
	uint64_t body = 1;
	y[task][id][0] = 2 % no;
	for (int i = 1; i < 32; i++)
	{
		y[task][id][i] = (y[task][id][i - 1] * y[task][id][i - 1]) % no;
	}
}
inline void PowerModulo(uint32_t no, uint32_t sPrime, uint8_t task, uint8_t id)
{
	uint32_t prime_2 = no;
	uint64_t mod = 1;
	for (uint32_t z = 0; prime_2 != 0; z++) {
		if (prime_2 & 1) { mod = (mod*(uint64_t)y[task][id][z]) % sPrime; }
		prime_2 = prime_2 >> 1;
	}
	if (mod == 1) {
		clrpixelprime(no);
		printf("\n delete 2^%u-1/%u \n", no, sPrime);
		mI.lock();
		ofstream file("PrimeRaport.txt", ios::app);
		if (file.good()) {
			file << no << " % " << sPrime << endl;
			file.close();
		}
		mI.unlock();

	}
}


inline void Negative(uint8_t task) {

	uint32_t j = 0;
	uint32_t t[MAX_S];
	while (index < MAX_PRIME - 1) {
		if (save) break;
		mI.lock();
		index += 2;
		sum++;

		if (index < MAX_PRIME - 1) {
			if (getpixel(index)) {
				clrpixel(index);
				t[j] = index;
				fillPowerModulo(task, j, index);
				j++;
				mI.unlock();
				if (j == MAX_S) {
					printf("%u ", t[MAX_S - 1]);
					j = 0;
					for (uint32_t i = 3; i < MAX_PRIME - 1; i += 2) {
						if (getpixelprime(i)) for (uint8_t l = 0; l < MAX_S; l++) PowerModulo(i, t[l], task, l);
					}
				}
			}
			else mI.unlock();
		}
		else mI.unlock();
	}
}
inline void Read() {
	cout << "read";
	ifstream file("prime", ios::binary);
	ifstream file2("arPrime", ios::binary);
	if (file.good() && file2.good()) {
		cout << " is progres";
		for (uint32_t i = 0; i < MAX_PRIME / 32 / 2; i++) {
			file.read((char*)&prime[i], 4);
			file2.read((char*)&arPrime[i], 4);
		}
		file.close();
		file2.close();
		cout << ". END read";
	}
	else  cout << " fail";
	cout << endl;
}
inline void Write() {
	cout << "write";
	uint32_t in = index;
	ofstream file("prime", ios::binary | ios::trunc);
	ofstream file2("arPrime", ios::binary | ios::trunc);
	ofstream file3("prime" + to_string(in), ios::binary | ios::trunc);
	ofstream file4("arPrime" + to_string(in), ios::binary | ios::trunc);
	if (file.good() && file2.good() && file3.good() && file4.good()) {
		cout << " is progres";
		for (uint32_t i = 0; i < MAX_PRIME / 32 / 2; i++) {
			file.write((const char*)&prime[i], 4);
			file2.write((const char*)&arPrime[i], 4);
			file3.write((const char*)&prime[i], 4);
			file4.write((const char*)&arPrime[i], 4);
		}
		file.close();
		file2.close();
		file3.close();
		file4.close();
		cout << ". END write";
	}
	else  cout << " fail";
	cout << endl;
}


int main()
{

	fill();
	Read();
	thread th[MAX_TASK];
	while (index < MAX_PRIME - 1) {
		for (uint8_t i = 0; i < MAX_TASK; i++) if (!th[i].joinable())th[i] = thread(Negative, i);
		if (sum > 10000000) {
			save = true;
			sum = 0;
			for (uint8_t i = 0; i < MAX_TASK; i++) if (th[i].joinable())th[i].join();
			Write();
			save = false;
		}
		if (GetAsyncKeyState(0x53)) {
			cout << "Press s. Soon save procedure will start";
			save = true;
			for (uint8_t i = 0; i < MAX_TASK; i++) if (th[i].joinable())th[i].join();
			Write();
			save = false;
		}
		this_thread::yield();
	}


	for (uint8_t i = 0; i < MAX_TASK; i++) if (th[i].joinable()) th[i].join();
	Write();
	int end;
	cout << " end ";
	cin >> end;

	return 0;
}

//basic code: 
// gcc primes.c -m64 -O3 -o primes && ./primes
//#include <fcntl.h>
//#include <stdio.h>
//#include <stdint.h>
//#include <unistd.h>
//
//#define MAX_PRIME (1ll<<32)
//#define SQRT_MAX_PRIME (1<<16)
//
//uint32_t x[MAX_PRIME / 32];
//
//inline unsigned getpixel(uint32_t i) { return (x[i >> 5] >> (i & 0x1F)) & 1; }
//inline void clrpixel(uint64_t i) { x[i >> 5] &= ~(1 << (i & 0x1F)); }
//
//void fill()
//{
//	uint64_t i, j;
//	for (i = 0; i<MAX_PRIME / 32; i++)
//		x[i] = ~0;
//
//	clrpixel(0);
//	for (j = 4; j<MAX_PRIME; j += 2) clrpixel(j);
//	for (j = 6; j<MAX_PRIME; j += 3) clrpixel(j);
//
//	for (i = 3; i<SQRT_MAX_PRIME; i++)
//	{
//		if (!getpixel(i)) continue;
//		printf("%i\r", i); fflush(stdout);
//		for (j = 3 * i; j<MAX_PRIME; j += 2 * i)
//			clrpixel(j);
//	}
//}
//
//int main()
//{
//	int fd;
//
//	fill();
//	fd = open("primes.bin", O_WRONLY | O_CREAT);
//	write(fd, x, sizeof(x));
//	close(fd);
//
//	return 0;
//}

