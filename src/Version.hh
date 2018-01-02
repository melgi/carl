//
// Copyright 2017 Giovanni Mels
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef CARL_VERSION_HH
#define CARL_VERSION_HH


#define CARL_MAJOR_VERSION     1
#define CARL_MINOR_VERSION     0
#define CARL_PATCH_VERSION     3
#define CARL_QUALIFIER_VERSION "SNAPSHOT"



#define CARL_STR(x) #x

#ifdef CARL_QUALIFIER_VERSION
#	define CARL_VERSION(MAJ, MIN, PATCH) CARL_STR(MAJ) "." CARL_STR(MIN) "." CARL_STR(PATCH) "-" CARL_QUALIFIER_VERSION
#else
#	define CARL_VERSION(MAJ, MIN, PATCH) CARL_STR(MAJ) "." CARL_STR(MIN) "." CARL_STR(PATCH)
#endif

#define CARL_VERSION_STR       CARL_VERSION(CARL_MAJOR_VERSION, CARL_MINOR_VERSION, CARL_PATCH_VERSION)

#endif /* CARL_VERSION_HH */
