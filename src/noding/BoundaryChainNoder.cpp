/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/BoundaryChainNoder.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/SegmentString.h>


using geos::geom::CoordinateSequence;
using geos::geom::Coordinate;


namespace geos {   // geos
namespace noding { // geos::noding

/* public */
void
BoundaryChainNoder::computeNodes(std::vector<SegmentString*>* segStrings)
{
    SegmentSet segSet;
    std::vector<BoundarySegmentMap> bdySections;
    bdySections.reserve(segStrings->size());
    addSegments(segStrings, segSet, bdySections);
    markBoundarySegments(segSet);
    chainList = extractChains(bdySections);
}

/* public */
std::vector<SegmentString*>*
BoundaryChainNoder::getNodedSubstrings() const
{
    return chainList;
}

/* private static */
void
BoundaryChainNoder::addSegments(
    std::vector<SegmentString*>* segStrings,
    SegmentSet& segSet,
    std::vector<BoundarySegmentMap>& includedSegs)
{
    for (SegmentString* ss : *segStrings) {
        includedSegs.emplace_back(ss);
        BoundarySegmentMap& segInclude = includedSegs.back();
        addSegments(ss, segInclude, segSet);
    }
}

/* private static */
bool
BoundaryChainNoder::segSetContains(SegmentSet& segSet, Segment& seg)
{
    auto search = segSet.find(seg);
    if(search != segSet.end()) {
        return true;
    }
    else {
        return false;
    }
}

/* private static */
void
BoundaryChainNoder::addSegments(
    SegmentString* segString,
    BoundarySegmentMap& segMap,
    SegmentSet& segSet)
{
    for (std::size_t i = 0; i < segString->size() - 1; i++) {
        const Coordinate& p0 = segString->getCoordinate(i);
        const Coordinate& p1 = segString->getCoordinate(i + 1);
        Segment seg(p0, p1, segMap, i);
        if (segSetContains(segSet, seg)) {
            segSet.erase(seg);
        }
        else {
            segSet.insert(seg);
        }
    }
}


/* private static */
void
BoundaryChainNoder::markBoundarySegments(SegmentSet& segSet)
{
    for (const Segment& seg : segSet) {
        seg.markInBoundary();
    }
}

/* private static */
std::vector<SegmentString*>*
BoundaryChainNoder::extractChains(std::vector<BoundarySegmentMap>& sections)
{
    std::vector<SegmentString*>* sectionList = new std::vector<SegmentString*>();
    for (BoundarySegmentMap& sect : sections) {
        sect.createChains(*sectionList);
    }
    return sectionList;
}

/*************************************************************************
 * BoundarySegmentMap
 */

/* public */
void
BoundaryChainNoder::BoundarySegmentMap::setBoundarySegment(std::size_t index)
{
    isBoundary[index] = true;
}

/* public */
void
BoundaryChainNoder::BoundarySegmentMap::createChains(
    std::vector<SegmentString*>& chains)
{
    std::size_t endIndex = 0;
    while (true) {
        std::size_t startIndex = findChainStart(endIndex);
        if (startIndex >= segString->size() - 1)
            break;
        endIndex = findChainEnd(startIndex);
        SegmentString* ss = createChain(segString, startIndex, endIndex);
        chains.push_back(ss);
    }
}


/* private static */
SegmentString*
BoundaryChainNoder::BoundarySegmentMap::createChain(
    const SegmentString* segString,
    std::size_t startIndex,
    std::size_t endIndex)
{
    std::unique_ptr<CoordinateSequence> pts(new CoordinateSequence());
    // Coordinate[] pts = new Coordinate[endIndex - startIndex + 1];
    for (std::size_t i = startIndex; i < endIndex + 1; i++) {
        pts->add(segString->getCoordinate(i));
    }
    return new NodedSegmentString(pts.release(), segString->getData());
}

/* private */
std::size_t
BoundaryChainNoder::BoundarySegmentMap::findChainStart(std::size_t index) const
{
    while (index < isBoundary.size() && ! isBoundary[index]) {
        index++;
    }
    return index;
}

/* private */
std::size_t
BoundaryChainNoder::BoundarySegmentMap::findChainEnd(std::size_t index) const
{
    index++;
    while (index < isBoundary.size() && isBoundary[index]) {
        index++;
    }
    return index;
}


} // geos::noding
} // geos
