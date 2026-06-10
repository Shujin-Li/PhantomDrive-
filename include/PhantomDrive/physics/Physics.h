#pragma once

/**
 * @file Physics.h
 * @brief 统一包含 MiniCore 物理引擎所有头文件
 * 
 * MiniCore 物理引擎直接从 DustRacing2D 复制，保持原始命名和实现
 * 所有类使用 MC 前缀（如 MCPhysicsComponent, MCVector2d 等）
 */

// 核心数学类型
#include "mcmacros.hh"
#include "mcvector2d.hh"
#include "mcvector3d.hh"
#include "mcbbox.hh"
#include "mcbbox3d.hh"
#include "mcobbox.hh"
#include "mctrigonom.hh"
#include "mcmathutil.hh"
#include "mclogger.hh"
#include "mcrandom.hh"

// 对象系统
#include "mcobjectdata.hh"
#include "mcobjectcomponent.hh"
#include "mcobject.hh"
#include "mctyperegistry.hh"
#include "mcobjectfactory.hh"

// 事件系统
#include "mcevent.hh"
#include "mctimerevent.hh"
#include "mccollisionevent.hh"
#include "mcseparationevent.hh"
#include "mcoutofboundariesevent.hh"

// 物理形状
#include "mcshape.hh"
#include "mccircleshape.hh"
#include "mcrectshape.hh"
#include "mcsegment.hh"
#include "mcedge.hh"

// 物理组件
#include "mcphysicscomponent.hh"

// 力系统
#include "mcforcegenerator.hh"
#include "mcforceregistry.hh"
#include "mcgravitygenerator.hh"
#include "mcfrictiongenerator.hh"
#include "mcdragforcegenerator.hh"
#include "mcimpulsegenerator.hh"
#include "mcspringforcegenerator.hh"
#include "mcspringforcegenerator2dfast.hh"

// 碰撞检测
#include "mccontact.hh"
#include "mccollisiondetector.hh"
#include "mcobjectgrid.hh"

// 物理世界
#include "mcworld.hh"

// 其他
#include "mcrecycler.hh"
#include "mcvectoranimation.hh"
#include "mccast.hh"
