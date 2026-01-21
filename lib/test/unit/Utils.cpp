#include <gtest/gtest.h>
#include "Utils.hpp"
#include "SimulationData.hpp"

TEST(ToArray, Base) {
    std::string t1("test");
    EXPECT_EQ(ToArray<2>(t1).at(0), 't');
    EXPECT_EQ(ToArray<5>(t1).at(1), 'e');
    EXPECT_EQ(ToArray<8>(t1).at(2), 's');
    EXPECT_EQ(ToArray<8>(t1).at(3), 't');
}

TEST(ToArray, NullTermination) {
    std::string t1("test");
    EXPECT_EQ(ToArray<5>(t1).at(4), 0);
    EXPECT_EQ(ToArray<2>(t1).at(1), 0);
    EXPECT_EQ(ToArray<8>(t1).at(7), 0);
    EXPECT_EQ(ToArray<8>(t1).at(4), 0);
}

// (A14) - wiek 4 - 12 lat z opiekunem, wzrost od 130 cm
TEST(MeetsAttractionCriteria, A14) {
    constexpr int attractionID = 13;

    PersonData parent{ 0, 0 };
    PersonData child{ 0, 0 };
    /// no child, invalid child data, invalid parent data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, false), false);
    /// has child, invalid child data, invalid parent data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, true), false);

    child = { 11, 140 };

    /// no child, valid child data, invalid parent data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, false), false);
    /// has child, valid child data, invalid parent data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, true), false);

    parent = { 30, 170 };    
    child = { 0, 0 };

    /// no child, invalid child data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, false), true);
    /// has child, invalid child data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, true), false);

    child = { 11, 140 };
    /// no child, valid child data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, false), true);
    /// has child, valid child data
    EXPECT_EQ(MeetsAttractionCriteria(attractionID, parent, child, true), true);
}

TEST(MeetsAttractionCriteria, InvalidAttractionID) {
    PersonData parent{ 0, 0 };
    PersonData child{ 0, 0 };

    EXPECT_NO_THROW(MeetsAttractionCriteria(-1, parent, child, false));
    EXPECT_NO_THROW(MeetsAttractionCriteria(AttractionConfig.size(), parent, child, false));
    EXPECT_NO_THROW(MeetsAttractionCriteria(AttractionConfig.size() + 1, parent, child, false));

    /// parent and child data that would match any attraction
    parent = { 30, 170 }; 
    child = { 11, 140 };

    EXPECT_EQ(MeetsAttractionCriteria(-1, parent, child, false), false);
    EXPECT_EQ(MeetsAttractionCriteria(AttractionConfig.size(), parent, child, false), false);
    EXPECT_EQ(MeetsAttractionCriteria(AttractionConfig.size() + 1, parent, child, false), false);
}