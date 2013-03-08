//-*****************************************************************************
//
// Copyright (c) 2013,
//  Sony Pictures Imageworks Inc. and
//  Industrial Light & Magic, a division of Lucasfilm Entertainment Company Ltd.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Sony Pictures Imageworks, nor
// Industrial Light & Magic, nor the names of their contributors may be used
// to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//-*****************************************************************************

#ifndef _Alembic_AbcCoreHDF5_Foundation_h_
#define _Alembic_AbcCoreHDF5_Foundation_h_

#include <Alembic/AbcCoreAbstract/All.h>

#include <Alembic/Util/All.h>

#include <Alembic/Ogawa/All.h>

#include <vector>
#include <string>
#include <map>

#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define ALEMBIC_OGAWA_FILE_VERSION 0

//-*****************************************************************************

namespace Alembic {
namespace AbcCoreOgawa {
namespace ALEMBIC_VERSION_NS {

//-*****************************************************************************
namespace AbcA = ::Alembic::AbcCoreAbstract;

using namespace ::Alembic::Util;
using AbcA::index_t;
using AbcA::chrono_t;

//-*****************************************************************************
typedef Alembic::Util::weak_ptr<AbcA::ObjectWriter> WeakOwPtr;
typedef Alembic::Util::weak_ptr<AbcA::BasePropertyWriter> WeakBpwPtr;

typedef Alembic::Util::weak_ptr<AbcA::ObjectReader> WeakOrPtr;
typedef Alembic::Util::weak_ptr<AbcA::BasePropertyReader> WeakBprPtr;

//-*****************************************************************************
struct PropertyHeaderAndFriends
{
    // for compounds
    PropertyHeaderAndFriends( const std::string &iName,
        const AbcA::MetaData &iMetaData ) :
        header( iName, iMetaData ),
    {
        isScalarLike = true;
        isHomogenous = true;
        nextSampleIndex = 0;
        firstChangedIndex = 0;
        lastChangedIndex = 0;
        timeSamplingIndex = 0;
    }

    // for scalar and array properties
    PropertyHeaderAndFriends( const std::string &iName,
        AbcA::PropertyType iPropType,
        const AbcA::MetaData &iMetaData,
        const AbcA::DataType &iDataType,
        const AbcA::TimeSamplingPtr & iTsamp,
        uint32_t iTimeSamplingIndex ) :
        header( iName, iPropType, iMetaData, iDataType, iTSamp ),
        timeSamplingIndex( iTimeSamplingIndex )
    {
        isScalarLike = true;
        isHomogenous = true;
        nextSampleIndex = 0;
        firstChangedIndex = 0;
        lastChangedIndex = 0;
    }

    // The header which defines this property.
    AbcA::PropertyHeader header;

    bool isScalarLike;

    bool isHomogenous;

    // Index of the next sample to write
    uint32_t nextSampleIndex;

    // Index representing the first sample that is different from sample 0
    uint32_t firstChangedIndex;

    // Index representing the last sample in which a change has occured
    // There is no need to repeat samples if they are the same between this
    // index and nextSampleIndex
    uint32_t lastChangedIndex;

    // Index representing which TimeSampling from the ArchiveWriter to use.
    uint32_t timeSamplingIndex;
};

typedef Alembic::Util::shared_ptr<PropertyHeaderAndFriends> PropertyHeaderPtr;
typedef std::vector<PropertyHeaderPtr> PropertyHeaderPtrs;

typedef Alembic::Util::shared_ptr<AbcA::ObjectHeader> ObjectHeaderPtr;

} // End namespace ALEMBIC_VERSION_NS

using namespace ALEMBIC_VERSION_NS;

} // End namespace AbcCoreOgawa
} // End namespace Alembic

#endif
