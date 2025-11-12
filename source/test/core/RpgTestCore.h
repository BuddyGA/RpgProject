#pragma once


namespace RpgTest
{
	namespace Core
	{
		extern void Test_Algorithm() noexcept;
		extern void Test_Array() noexcept;
		extern void Test_FreeList() noexcept;
		extern void Test_Map() noexcept;
		extern void Test_String() noexcept;
		extern void Test_FilePath() noexcept;
		extern void Test_Pointer() noexcept;


		inline void Execute() noexcept
		{
			Test_Algorithm();
			Test_Array();
			Test_FreeList();
			Test_Map();
			Test_String();
			Test_FilePath();
			Test_Pointer();
		}

	};

};
