#pragma once
#pragma warning( disable : 4996)
#pragma warning( disable : 4011)

extern "C" int _fltused = 0;

// default
#include <ntddk.h>
#include <cstdint>
#include <ntddmou.h>

// impl
#include <impl/std/std.hxx>
#include <impl/ia32/ia32.hxx>
#include <impl/oxorany/oxorany.hxx>
#include <impl/nt/ntoskrnl.hxx>

// resolver
#include <workspace/utility/resolver/exports.hxx>

// utility
#include <workspace/utility/primitives.hxx>
#include <workspace/utility/common.hxx>

// requests
#include <workspace/core/control/requests.hxx>

// memory
#include <workspace/core/mm/phyiscal.hxx>
#include <workspace/core/mm/paging.hxx>
#include <workspace/core/mm/gadget.hxx>

// device
#include <workspace/core/device/mouse.hxx>
#include <workspace/core/device/irp.hxx>

// callbacks / dispatch
#include <workspace/core/control/batching.hxx>
#include <workspace/core/control/dispatch.hxx>
#include <workspace/core/callbacks/subscription.hxx>