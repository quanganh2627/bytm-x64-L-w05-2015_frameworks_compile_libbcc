/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BCC_USC_ABC_COMPILER_DRIVER_H
#define BCC_USC_ABC_COMPILER_DRIVER_H

#include "bcc/AndroidBitcode/ABCCompilerDriver.h"

namespace bcc {

class USCABCCompilerDriver : public ABCCompilerDriver {
public:
  USCABCCompilerDriver()
    : ABCCompilerDriver() { }

  virtual ~USCABCCompilerDriver() { }

private:
  virtual CompilerConfig *createCompilerConfig() const;
  virtual LinkerConfig *createLinkerConfig() const;

  virtual ABCExpandVAArgPass *createExpandVAArgPass() const;
};

} // end namespace bcc

#endif // BCC_USC_ABC_COMPILER_DRIVER_H
