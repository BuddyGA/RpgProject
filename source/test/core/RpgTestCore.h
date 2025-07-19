#pragma once


namespace RpgTest
{
	namespace Core
	{
		extern void Test_DSA_Algorithm() noexcept;
		extern void Test_DSA_Array() noexcept;
		extern void Test_DSA_FreeList() noexcept;
		extern void Test_DSA_Map() noexcept;
		extern void Test_String() noexcept;
		extern void Test_FilePath() noexcept;
		extern void Test_Pointer() noexcept;


		inline void Execute() noexcept
		{
			Test_DSA_Algorithm();
			Test_DSA_Array();
			Test_DSA_FreeList();
			Test_DSA_Map();
			Test_String();
			Test_FilePath();
			Test_Pointer();
		}

	};

};
