#pragma once

/*
FileMasksProcessor.hpp

Класс для работы с простыми масками файлов (не учитывается наличие масок
исключения).
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <vector>
#include "noncopyable.hpp"
#include "RegExp.hpp"
#include "FARString.hpp"

#define EXCLUDEMASKSEPARATOR (L'|')

enum FM_FLAGS
{
	FMF_SILENT      = 0x00000001,
	FMF_ADDASTERISK = 0x00000002, // Добавлять '*', если маска не содержит ни одного из символов: '*', '?', '.'
};

class BaseFileMask : private NonCopyable
{
	public:
		BaseFileMask() {}
		virtual ~BaseFileMask() {}

	public:
		virtual bool Set(const wchar_t *Masks, DWORD Flags) = 0;
		virtual bool Compare(const wchar_t *Name, bool ignorecase) const = 0;
		virtual void Reset() = 0;
};

class SingleFileMask : public BaseFileMask
{
	public:
		SingleFileMask() : BaseFileMask() {}
		~SingleFileMask() override {}

	public:
		bool Set(const wchar_t *Masks, DWORD Flags) override;
		bool Compare(const wchar_t *Name, bool ignorecase) const override;
		void Reset() override;

	private:
		FARString Mask;
};

class RegexMask : public BaseFileMask
{
	public:
		RegexMask();
		~RegexMask() override {}

	public:
		bool Set(const wchar_t *Masks, DWORD Flags) override;
		bool Compare(const wchar_t *Name, bool ignorecase) const override;
		void Reset() override;

	private:
		std::unique_ptr<RegExp> re;
};

class FileMasksProcessor : public BaseFileMask
{
	public:
		FileMasksProcessor();
		~FileMasksProcessor() override { Reset(); }

	public:
		bool Set(const wchar_t *Masks, DWORD Flags) override;
		bool Compare(const wchar_t *Name, bool ignorecase) const override;
		void Reset() override;

	private:
		std::vector<BaseFileMask*> IncludeMasks;
		std::vector<BaseFileMask*> ExcludeMasks;
		const int CallDepth;

	private:
		FileMasksProcessor(int aCallDepth);
		bool SetPart(const wchar_t *Masks, DWORD Flags, std::vector<BaseFileMask*>& Target);
};

