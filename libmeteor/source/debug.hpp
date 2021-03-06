// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>

// for abort macro
#include "ameteor.hpp"

// from cinterface.cpp
void print_bizhawk(const char *msg);
void print_bizhawk(std::string &msg);
void abort_bizhawk(const char *msg);
void keyupdate_bizhawk();
extern bool traceenabled;
void trace_bizhawk(std::string msg);
extern int slcallbackline;
void scanlinecallback_bizhawk();

#if 0
#define met_abort(str) \
	{ \
		std::cerr << IOS_NOR << "Fatal error :\n" << str << "\nFile : " \
		<< __FILE__ << "\nLine : " << __LINE__ << "\nr15 = " \
		<< IOS_ADD << ::AMeteor::_cpu.Reg(15) << "\n[r15] = " << IOS_ADD \
		<< ::AMeteor::_memory.Read32(::AMeteor::_cpu.Reg(15)) \
		<< "\nFlag T : " << ::AMeteor::_cpu.ICpsr().thumb << std::endl; \
		abort(); \
	}
#endif
#ifdef METDEBUG
#include <sstream>
//extern "C" int __stdcall MessageBoxA(int, const char *, const char *, int);
#define met_abort(_str) if(true)\
	{ \
		std::stringstream _zisrny; \
		_zisrny << IOS_NOR << "Fatal error :\n" << _str << "\nFile : " \
		<< __FILE__ << "\nLine : " << __LINE__ << "\nr15 = " \
		<< IOS_ADD << ::AMeteor::_cpu.Reg(15) << "\n[r15] = " << IOS_ADD \
		<< ::AMeteor::_memory.Read32(::AMeteor::_cpu.Reg(15)) \
		<< "\nFlag T : " << ::AMeteor::_cpu.ICpsr().thumb << std::endl; \
		abort_bizhawk(_zisrny.str().c_str()); \
	}

#else
#define met_abort(str) {}
#endif

#define STDBG std::cout
//#define STDBG debug_stream

#if defined METDEBUG && defined METDEBUGLOG
//XXX
#	define MYDEBUG
#	define debug(_str) \
	{ \
		std::stringstream _zisrny; \
		_zisrny << _str << std::endl; \
		print_bizhawk(_zisrny.str()); \
	}
		//STDBG << str << std::endl
//#	define debug_(str) \
//		STDBG << str
#else
#	define debug(s) {}
//#	define debug_(s) {}
#endif

#define IOS_ADD \
	"0x" << std::setbase(16) << std::setw(8) << std::setfill('0') \
		<< std::uppercase
#define IOS_TRACE \
		std::setbase(16) << std::setw(8) << std::setfill('0') << std::uppercase
#define IOS_NOR \
	std::setbase(10) << std::setw(0) << std::setfill(' ')

namespace AMeteor
{
	void debug_bits(uint32_t u);
	void debug_bits_16(uint16_t u);

#if defined MET_REGS_DEBUG
	void PrintRegs ();
	void PrintStack (uint32_t stackadd);
#else
#define PrintRegs()
#define PrintStack(b)
#endif
}

#endif
