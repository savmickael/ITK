/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#include <vxl_version.h>
#if VXL_VERSION_DATE_FULL < 20120316
#ifdef _WIN64
#include <vnl/vnl_matrix.txx>
VNL_MATRIX_INSTANTIATE(long long);
#endif
#else
void vnl_matrix_longlong_dummy(void) {}
#endif
