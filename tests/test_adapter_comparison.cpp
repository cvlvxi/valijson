#include <picojson.h>

#include <gtest/gtest.h>

#include <valijson/adapters/json11_adapter.hpp>
#include <valijson/adapters/jsoncpp_adapter.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/adapters/picojson_adapter.hpp>
#include <valijson/adapters/rapidjson_adapter.hpp>

#include <valijson/utils/json11_utils.hpp>
#include <valijson/utils/jsoncpp_utils.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>
#include <valijson/utils/picojson_utils.hpp>
#include <valijson/utils/rapidjson_utils.hpp>
#include <valijson/utils/experimental_utils.hpp>

#ifdef VALIJSON_BUILD_PROPERTY_TREE_ADAPTER
#include <valijson/adapters/property_tree_adapter.hpp>
#include <valijson/utils/property_tree_utils.hpp>
#endif

#ifdef VALIJSON_BUILD_QT_ADAPTER
#include <utility>
#include <valijson/adapters/qtjson_adapter.hpp>
#include <valijson/utils/qtjson_utils.hpp>
#endif

#ifdef VALIJSON_BUILD_POCO_ADAPTER
#include <valijson/adapters/poco_json_adapter.hpp>
#include <valijson/utils/poco_json_utils.hpp>
#endif

#define TEST_DATA_DIR "../tests/data/documents/"

using valijson::adapters::AdapterTraits;

class TestAdapterComparison : public testing::Test {
protected:
  struct JsonFile {
    JsonFile(std::string path, int strictGroup, int looseGroup)
        : m_path(std::move(path)), m_strictGroup(strictGroup),
          m_looseGroup(looseGroup) {}

    std::string m_path;
    int m_strictGroup;
    int m_looseGroup;
  };
  static std::vector<JsonFile> s_jsonFiles;

  static void SetUpTestCase() {
    const std::string testDataDir(TEST_DATA_DIR);

    //
    // Each test is allocated to two groups. The first group is the strict
    // comparison group. All test files that have been assigned to the same
    // group should be equal, when compared using strict types. The second
    // group is the loose comparison group. All tests files in a loose
    // group should be equal, when compared without using strict types.
    //
    // As an example, the first three test files are in the same loose
    // group. This means they are expected to be equal when compared without
    // strict types. However, only the first two files in the same strict
    // group, which means that only they should be equal.
    //
    s_jsonFiles.emplace_back(testDataDir + "array_doubles_1_2_3.json", 1, 1);
    s_jsonFiles.emplace_back(testDataDir + "array_integers_1_2_3.json", 1, 1);
    s_jsonFiles.emplace_back(testDataDir + "array_strings_1_2_3.json", 2, 1);

    s_jsonFiles.emplace_back(testDataDir + "array_doubles_1_2_3_4.json", 3, 2);
    s_jsonFiles.emplace_back(testDataDir + "array_integers_1_2_3_4.json", 3, 2);
    s_jsonFiles.emplace_back(testDataDir + "array_strings_1_2_3_4.json", 4, 2);

    s_jsonFiles.emplace_back(testDataDir + "array_doubles_10_20_30_40.json", 5,
                             3);
    s_jsonFiles.emplace_back(testDataDir + "array_integers_10_20_30_40.json", 5,
                             3);
    s_jsonFiles.emplace_back(testDataDir + "array_strings_10_20_30_40.json", 6,
                             3);
  }

  // Make a function that takes a vector and another function
  // Pairwise iterate

  template <typename Callback> static void pairwiseTest(Callback callback) {
    for (auto outerItr = s_jsonFiles.begin(); outerItr != s_jsonFiles.end() - 1;
         ++outerItr) {
      for (auto innerItr = outerItr; innerItr != s_jsonFiles.end();
           ++innerItr) {
        callback(*outerItr, *innerItr);
      }
    }
  }

  template <typename Adapter1, typename Adapter2> static void testComparison() {
    std::vector<JsonFile>::const_iterator outerItr, innerItr;

    pairwiseTest([](const JsonFile &item1, const JsonFile &item2) {
      const bool expectedStrict = (item1.m_strictGroup == item2.m_strictGroup);
      const bool expectedLoose = (item1.m_looseGroup == item2.m_looseGroup);

      using DocumentType1 = typename AdapterTraits<Adapter1>::DocumentType;
      DocumentType1 document1;
      ASSERT_TRUE(valijson::utils::loadDocument(item1.m_path, document1));
      optional<DocumentType1> result = valijson::utils::loadDocumentExperimental<DocumentType1>(item1.m_path);
      ASSERT_TRUE(bool(result));

      const Adapter1 adapter1(document1);
      const std::string adapter1Name = AdapterTraits<Adapter1>::adapterName();

      typename AdapterTraits<Adapter2>::DocumentType document2;
      ASSERT_TRUE(valijson::utils::loadDocument(item2.m_path, document2));
      const Adapter2 adapter2(document2);
      const std::string adapter2Name = AdapterTraits<Adapter2>::adapterName();

      // If either adapter does not support strict types, then strict
      // comparison should not be used, UNLESS the adapters are of the
      // same type. If they are of the same type, then the internal
      // type degradation should be the same, therefore strict testing
      // of equality makes sense.
      if (adapter1.hasStrictTypes() && adapter2.hasStrictTypes() &&
          adapter1Name == adapter2Name) {
        EXPECT_EQ(expectedStrict, adapter1.equalTo(adapter2, true))
            << "Comparing '" << item1.m_path << "' to '" << item2.m_path << "' "
            << "with strict comparison enabled";
        EXPECT_EQ(expectedStrict, adapter2.equalTo(adapter1, true))
            << "Comparing '" << item2.m_path << "' to '" << item1.m_path << "' "
            << "with strict comparison enabled";
      }

      EXPECT_EQ(expectedLoose, adapter1.equalTo(adapter2, false))
          << "Comparing '" << item1.m_path << "' to '" << item2.m_path << "' "
          << "with strict comparison disabled";
      EXPECT_EQ(expectedLoose, adapter2.equalTo(adapter1, false))
          << "Comparing '" << item2.m_path << "' to '" << item1.m_path << "' "
          << "with strict comparison disabled";
    });
  }
};

std::vector<TestAdapterComparison::JsonFile> TestAdapterComparison::s_jsonFiles;

//
// JsonCppAdapter vs X
// ------------------------------------------------------------------------------------------------

TEST_F(TestAdapterComparison, JsonCppVsJsonCpp) {
  testComparison<valijson::adapters::JsonCppAdapter,
                 valijson::adapters::JsonCppAdapter>();
}

TEST_F(TestAdapterComparison, JsonCppVsPicoJson) {
  testComparison<valijson::adapters::JsonCppAdapter,
                 valijson::adapters::PicoJsonAdapter>();
}


//
// PicoJsonAdapter vs X
// ------------------------------------------------------------------------------------------------

TEST_F(TestAdapterComparison, PicoJsonVsPicoJson) {
  testComparison<valijson::adapters::PicoJsonAdapter,
                 valijson::adapters::PicoJsonAdapter>();
}


//
// Json11Adapter vs X
// ------------------------------------------------------------------------------------------------

TEST_F(TestAdapterComparison, Json11VsJson11) {
  testComparison<valijson::adapters::Json11Adapter,
                 valijson::adapters::Json11Adapter>();
}

TEST_F(TestAdapterComparison, Json11VsJsonCpp) {
  testComparison<valijson::adapters::Json11Adapter,
                 valijson::adapters::JsonCppAdapter>();
}




TEST_F(TestAdapterComparison, Json11VsPicoJson) {
  testComparison<valijson::adapters::Json11Adapter,
                 valijson::adapters::PicoJsonAdapter>();
}


//
// NlohmannJsonAdapter vs X
// ------------------------------------------------------------------------------------------------

TEST_F(TestAdapterComparison, NlohmannJsonVsNlohmannJson) {
  testComparison<valijson::adapters::NlohmannJsonAdapter,
                 valijson::adapters::NlohmannJsonAdapter>();
}

TEST_F(TestAdapterComparison, NlohmannJsonVsJson11) {
  testComparison<valijson::adapters::NlohmannJsonAdapter,
                 valijson::adapters::Json11Adapter>();
}

TEST_F(TestAdapterComparison, NlohmannJsonVsJsonCpp) {
  testComparison<valijson::adapters::NlohmannJsonAdapter,
                 valijson::adapters::JsonCppAdapter>();
}



TEST_F(TestAdapterComparison, NlohmannJsonVsPicoJson) {
  testComparison<valijson::adapters::NlohmannJsonAdapter,
                 valijson::adapters::PicoJsonAdapter>();
}

//
// PocoJsonAdapter vs X
// ------------------------------------------------------------------------------------------------

#ifdef VALIJSON_BUILD_POCO_ADAPTER

TEST_F(TestAdapterComparison, PocoJsonVsPocoJson) {
  testComparison<valijson::adapters::PocoJsonAdapter,
                 valijson::adapters::PocoJsonAdapter>();
}

TEST_F(TestAdapterComparison, PocoJsonVsJsonCpp) {
  testComparison<valijson::adapters::PocoJsonAdapter,
                 valijson::adapters::JsonCppAdapter>();
}



TEST_F(TestAdapterComparison, PocoJsonVsPicoJson) {
  testComparison<valijson::adapters::PocoJsonAdapter,
                 valijson::adapters::PicoJsonAdapter>();
}


TEST_F(TestAdapterComparison, PocoJsonVsJson11) {
  testComparison<valijson::adapters::PocoJsonAdapter,
                 valijson::adapters::Json11Adapter>();
}

TEST_F(TestAdapterComparison, PocoJsonVsNlohmannJsonAdapter) {
  testComparison<valijson::adapters::PocoJsonAdapter,
                 valijson::adapters::NlohmannJsonAdapter>();
}

#endif // VALIJSON_BUILD_POCO_ADAPTER
