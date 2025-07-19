#include "RpgTestCore.h"
#include "core/dsa/RpgAlgorithm.h"



static void Test_Swap() noexcept
{
	RpgPointInt a(11, 32);
	RpgPointInt b(22, 44);
	RpgAlgorithm::Swap(a, b);
	RPG_Assert(a.X == 22 && a.Y == 44);
	RPG_Assert(b.X == 11 && b.Y == 32);
}


static void Test_PowerOfTwo() noexcept
{
	const int16_t t0 = 256;
	RPG_Assert(RpgAlgorithm::IsPowerOfTwo(t0));

	const int16_t t1 = 1024;
	RPG_Assert(RpgAlgorithm::IsPowerOfTwo(t1));

	const uint16_t t2 = 2048;
	RPG_Assert(RpgAlgorithm::IsPowerOfTwo(t2));

	const uint16_t t3 = 300;
	RPG_Assert(!RpgAlgorithm::IsPowerOfTwo(t3));

	const int t4 = 64;
	RPG_Assert(RpgAlgorithm::IsPowerOfTwo(t4));

	const int t5 = 256;
	RPG_Assert(RpgAlgorithm::IsPowerOfTwo(t5));

	const int t6 = 120;
	RPG_Assert(!RpgAlgorithm::IsPowerOfTwo(t6));

	const uint32_t t7 = 1024;
	RPG_Assert(RpgAlgorithm::IsPowerOfTwo(t7));

	const uint32_t t8 = 900;
	RPG_Assert(!RpgAlgorithm::IsPowerOfTwo(t8));

	const uint32_t t9 = 1920;
	RPG_Assert(!RpgAlgorithm::IsPowerOfTwo(t9));
}


static void Test_Array_CopyElements() noexcept
{
	constexpr int COUNT = 16;

	int test0Src[COUNT] = { 3, 5, 6 };
	int test0Dst[COUNT] = { 1, 2, 4 };
	RpgAlgorithm::Array_CopyElements(3, test0Dst, COUNT, 3, test0Src, COUNT, 0);
	RPG_Assert(test0Dst[0] == 1);
	RPG_Assert(test0Dst[1] == 2);
	RPG_Assert(test0Dst[2] == 4);
	RPG_Assert(test0Dst[3] == 3);
	RPG_Assert(test0Dst[4] == 5);
	RPG_Assert(test0Dst[5] == 6);

	int test1Src[COUNT] = { 4, 8 };
	int test1Dst[COUNT] = { 0, 0, 3, 4, 5 };
	RpgAlgorithm::Array_CopyElements(2, test1Dst, COUNT, 0, test1Src, COUNT, 0);
	RPG_Assert(test1Dst[0] == 4);
	RPG_Assert(test1Dst[1] == 8);
	RPG_Assert(test1Dst[2] == 3);
	RPG_Assert(test1Dst[3] == 4);
	RPG_Assert(test1Dst[4] == 5);

	int test2Src[COUNT] = { 4, 5, 6, 7, 8, 9 };
	int test2Dst[COUNT] = { 11, 12, 0, 0, 0, 14, 15 };
	RpgAlgorithm::Array_CopyElements(3, test2Dst, COUNT, 2, test2Src, COUNT, 2);
	RPG_Assert(test2Dst[0] == 11);
	RPG_Assert(test2Dst[1] == 12);
	RPG_Assert(test2Dst[2] == 6);
	RPG_Assert(test2Dst[3] == 7);
	RPG_Assert(test2Dst[4] == 8);
	RPG_Assert(test2Dst[5] == 14);
	RPG_Assert(test2Dst[6] == 15);
}


static void Test_Array_ShiftElements() noexcept
{
	constexpr int COUNT = 16;

	int test0[COUNT] = { 0, 1, 2 };
	RpgAlgorithm::Array_ShiftElements(test0, COUNT, 0, 1);
	// [0] can be garbage
	RPG_Assert(test0[1] == 0);
	RPG_Assert(test0[2] == 1);
	RPG_Assert(test0[3] == 2);

	RpgAlgorithm::Array_ShiftElements(test0, COUNT, 1, 0);
	RPG_Assert(test0[0] == 0);
	RPG_Assert(test0[1] == 1);
	RPG_Assert(test0[2] == 2);


	int test1[COUNT] = { 3, 4, 5, 6, 7 };
	RpgAlgorithm::Array_ShiftElements(test1, COUNT, 1, 3);
	RPG_Assert(test1[0] == 3);
	// [1] can be garbage
	// [2] can be garbage
	RPG_Assert(test1[3] == 4);
	RPG_Assert(test1[4] == 5);
	RPG_Assert(test1[5] == 6);
	RPG_Assert(test1[6] == 7);

	RpgAlgorithm::Array_ShiftElements(test1, COUNT, 3, 1);
	RPG_Assert(test1[0] == 3);
	RPG_Assert(test1[1] == 4);
	RPG_Assert(test1[2] == 5);
	RPG_Assert(test1[3] == 6);
	RPG_Assert(test1[4] == 7);


	int test2[COUNT] = { 1, 2, 3 };
	RpgAlgorithm::Array_ShiftElements(test2, COUNT, 0, 6);
	// [0] can be garbage
	// [1] can be garbage
	// [2] can be garbage
	// [3] can be garbage
	// [4] can be garbage
	// [5] can be garbage
	RPG_Assert(test2[6] == 1);
	RPG_Assert(test2[7] == 2);
	RPG_Assert(test2[8] == 3);

	RpgAlgorithm::Array_ShiftElements(test2, COUNT, 6, 0);
	RPG_Assert(test2[0] == 1);
	RPG_Assert(test2[1] == 2);
	RPG_Assert(test2[2] == 3);
}


static void Test_Array_RemoveElements() noexcept
{
	constexpr int COUNT = 16;

	int test0[COUNT] = { 0, 1, 2 };
	RpgAlgorithm::Array_RemoveElements(test0, COUNT, 0, 1, true);
	RPG_Assert(test0[0] == 1);
	RPG_Assert(test0[1] == 2);


	int test1[COUNT] = { 3, 4, 5, 6, 7 };
	RpgAlgorithm::Array_RemoveElements(test1, COUNT, 1, 3, true);
	RPG_Assert(test1[0] == 3);
	RPG_Assert(test1[1] == 7);


	int test2[COUNT] = { 8, 7, 5 };
	RpgAlgorithm::Array_RemoveElements(test2, COUNT, 3, 1, true);
	RPG_Assert(test2[0] == 8);
	RPG_Assert(test2[1] == 7);


	int test3[COUNT] = { 1, 2, 3, 4, 5, 6, 7 };
	RpgAlgorithm::Array_RemoveElements(test3, COUNT, 0, 4, true);
	RPG_Assert(test3[0] == 5);
	RPG_Assert(test3[1] == 6);
	RPG_Assert(test3[2] == 7);
}


static void Test_Array_InsertElements() noexcept
{
	constexpr int COUNT = 16;

	int test0Src[COUNT] = { 5 };
	int test0Dst[COUNT] = { 7, 1, 2, 3 };
	RpgAlgorithm::Array_InsertElements(test0Dst, COUNT, 0, test0Src, 1);
	RPG_Assert(test0Dst[0] == 5);
	RPG_Assert(test0Dst[1] == 7);
	RPG_Assert(test0Dst[2] == 1);
	RPG_Assert(test0Dst[3] == 2);
	RPG_Assert(test0Dst[4] == 3);


	int test1Src[COUNT] = { 2, 3, 4 };
	int test1Dst[COUNT] = { 7, 8, 9 };
	RpgAlgorithm::Array_InsertElements(test1Dst, COUNT, 0, test1Src, 3);
	RPG_Assert(test1Dst[0] == 2);
	RPG_Assert(test1Dst[1] == 3);
	RPG_Assert(test1Dst[2] == 4);
	RPG_Assert(test1Dst[3] == 7);
	RPG_Assert(test1Dst[4] == 8);
	RPG_Assert(test1Dst[5] == 9);


	int test2Src[COUNT] = { 8 };
	int test2Dst[COUNT] = { 1, 2, 3, 4 };
	RpgAlgorithm::Array_InsertElements(test2Dst, COUNT, 4, test2Src, 1);
	RPG_Assert(test2Dst[0] == 1);
	RPG_Assert(test2Dst[1] == 2);
	RPG_Assert(test2Dst[2] == 3);
	RPG_Assert(test2Dst[3] == 4);
	RPG_Assert(test2Dst[4] == 8);


	int test3Src[COUNT] = { 8, 7, 4, 3 };
	int test3Dst[COUNT] = { 1, 2, 2 };
	RpgAlgorithm::Array_InsertElements(test3Dst, COUNT, 3, test3Src, 4);
	RPG_Assert(test3Dst[0] == 1);
	RPG_Assert(test3Dst[1] == 2);
	RPG_Assert(test3Dst[2] == 2);
	RPG_Assert(test3Dst[3] == 8);
	RPG_Assert(test3Dst[4] == 7);
	RPG_Assert(test3Dst[5] == 4);
	RPG_Assert(test3Dst[6] == 3);


	int test4Src[COUNT] = { 3 };
	int test4Dst[COUNT] = { 1, 2, 5, 6 };
	RpgAlgorithm::Array_InsertElements(test4Dst, COUNT, 2, test4Src, 1);
	RPG_Assert(test4Dst[0] == 1);
	RPG_Assert(test4Dst[1] == 2);
	RPG_Assert(test4Dst[2] == 3);
	RPG_Assert(test4Dst[3] == 5);
	RPG_Assert(test4Dst[4] == 6);


	int test5Src[COUNT] = { 1, 2, 3 };
	int test5Dst[COUNT] = { 8, 6, 7, 4, 11, 12, 13, 14 };
	RpgAlgorithm::Array_InsertElements(test5Dst, COUNT, 4, test5Src, 3);
	RPG_Assert(test5Dst[0] == 8);
	RPG_Assert(test5Dst[1] == 6);
	RPG_Assert(test5Dst[2] == 7);
	RPG_Assert(test5Dst[3] == 4);
	RPG_Assert(test5Dst[4] == 1);
	RPG_Assert(test5Dst[5] == 2);
	RPG_Assert(test5Dst[6] == 3);
	RPG_Assert(test5Dst[7] == 11);
	RPG_Assert(test5Dst[8] == 12);
	RPG_Assert(test5Dst[9] == 13);
	RPG_Assert(test5Dst[10] == 14);
}


void RpgTest::Core::Test_DSA_Algorithm() noexcept
{
	Test_Swap();
	Test_PowerOfTwo();
	Test_Array_CopyElements();
	Test_Array_ShiftElements();
	Test_Array_RemoveElements();
	Test_Array_InsertElements();
}
