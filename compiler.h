/*
 * compiler.h
 *
 *  Created on: 2013年7月18日
 *      Author: liutos
 */

#ifndef COMPILER_H_
#define COMPILER_H_

#include "type.h"

extern lt *assemble(lt *);
extern lt *compile_object(lt *, lt *);
extern lt *compile_to_bytecode(lt *);
extern lt *gen(enum TYPE, ...);

#endif /* COMPILER_H_ */
