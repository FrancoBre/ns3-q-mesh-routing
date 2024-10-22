/*
 * Copyright (c) 2023
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/he-phy.h"
#include "ns3/he-ppdu.h"
#include "ns3/test.h"
#include "ns3/wifi-phy-operating-channel.h"
#include "ns3/wifi-psdu.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiOperatingChannelTest");

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Test the WifiPhyOperatingChannel::Set() method.
 */
class SetWifiOperatingChannelTest : public TestCase
{
  public:
    /**
     * Constructor
     */
    SetWifiOperatingChannelTest();
    ~SetWifiOperatingChannelTest() override = default;

  private:
    void DoRun() override;

    /**
     * Run one function.
     * \param runInfo the string that indicates info about the test case to run
     * \param segments the info about each frequency segment to set for the operating channel
     * \param standard the 802.11 standard to consider for the test
     * \param band the PHY band to consider for the test
     * \param expectExceptionThrown flag to indicate whether an exception is expected to be thrown
     * \param expectedWidth the expected width type of the operating channel
     * \param expectedSegments the info about the expected frequency segments of the operating
     * channel
     */
    void RunOne(const std::string& runInfo,
                const std::vector<FrequencyChannelInfo>& segments,
                WifiStandard standard,
                WifiPhyBand band,
                bool expectExceptionThrown,
                WifiChannelWidthType expectedWidth = WifiChannelWidthType::UNKNOWN,
                const std::vector<FrequencyChannelInfo>& expectedSegments = {});

    WifiPhyOperatingChannel m_channel; //!< operating channel
};

SetWifiOperatingChannelTest::SetWifiOperatingChannelTest()
    : TestCase("Check configuration of the operating channel")
{
}

void
SetWifiOperatingChannelTest::RunOne(const std::string& runInfo,
                                    const std::vector<FrequencyChannelInfo>& segments,
                                    WifiStandard standard,
                                    WifiPhyBand band,
                                    bool expectExceptionThrown,
                                    WifiChannelWidthType expectedWidth,
                                    const std::vector<FrequencyChannelInfo>& expectedSegments)
{
    NS_LOG_FUNCTION(this << runInfo);

    bool exceptionThrown = false;
    try
    {
        m_channel.Set(segments, standard);
    }
    catch (const std::runtime_error&)
    {
        exceptionThrown = true;
    }
    NS_TEST_ASSERT_MSG_EQ(exceptionThrown,
                          expectExceptionThrown,
                          "Exception thrown mismatch for run: " << runInfo);
    if (!exceptionThrown)
    {
        NS_TEST_ASSERT_MSG_EQ(
            m_channel.GetWidthType(),
            expectedWidth,
            "Operating channel has an incorrect channel width type for run: " << runInfo);
        NS_TEST_ASSERT_MSG_EQ(m_channel.GetNSegments(),
                              expectedSegments.size(),
                              "Incorrect number of frequency segments for run: " << runInfo);
        for (std::size_t i = 0; i < m_channel.GetNSegments(); ++i)
        {
            const auto& frequencyChannelInfo = expectedSegments.at(i);
            NS_TEST_ASSERT_MSG_EQ(m_channel.GetNumber(i),
                                  frequencyChannelInfo.number,
                                  "Operating channel has an incorrect channel number at segment "
                                      << i << " for run: " << runInfo);
            NS_TEST_ASSERT_MSG_EQ(m_channel.GetFrequency(i),
                                  frequencyChannelInfo.frequency,
                                  "Operating channel has an incorrect center frequency at segment "
                                      << i << " for run: " << runInfo);
            NS_TEST_ASSERT_MSG_EQ(m_channel.GetWidth(i),
                                  frequencyChannelInfo.width,
                                  "Operating channel has an incorrect channel width at segment "
                                      << i << " for run: " << runInfo);
            NS_TEST_ASSERT_MSG_EQ(m_channel.GetPhyBand(),
                                  frequencyChannelInfo.band,
                                  "Operating channel has an incorrect band for run: " << runInfo);
        }
    }
}

void
SetWifiOperatingChannelTest::DoRun()
{
    RunOne("dummy channel with all inputs unset",
           {{}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           true);

    RunOne("default 20 MHz OFDM channel operating on channel 36",
           {{36, 0, 20, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           false,
           WifiChannelWidthType::CW_20MHZ,
           {{36, 5180, 20, WIFI_PHY_BAND_5GHZ, FrequencyChannelType::OFDM}});

    RunOne("default 40 MHz OFDM channel operating on channel 38",
           {{38, 0, 40, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           false,
           WifiChannelWidthType::CW_40MHZ,
           {{38, 5190, 40, WIFI_PHY_BAND_5GHZ, FrequencyChannelType::OFDM}});

    RunOne("default 80 MHz OFDM channel operating on channel 42",
           {{42, 0, 80, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           false,
           WifiChannelWidthType::CW_80MHZ,
           {{42, 5210, 80, WIFI_PHY_BAND_5GHZ, FrequencyChannelType::OFDM}});

    RunOne("default 160 MHz (contiguous) OFDM channel operating on channel 50",
           {{50, 0, 160, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           false,
           WifiChannelWidthType::CW_160MHZ,
           {{50, 5250, 160, WIFI_PHY_BAND_5GHZ, FrequencyChannelType::OFDM}});

    RunOne("valid 80+80 MHz (non-contiguous) OFDM channel operating on channels 42 and 106",
           {{42, 0, 80, WIFI_PHY_BAND_5GHZ}, {106, 0, 80, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           false,
           WifiChannelWidthType::CW_80_PLUS_80MHZ,
           {{42, 5210, 80, WIFI_PHY_BAND_5GHZ, FrequencyChannelType::OFDM},
            {106, 5530, 80, WIFI_PHY_BAND_5GHZ, FrequencyChannelType::OFDM}});

    RunOne("invalid 80+80 MHz (non-contiguous) OFDM channel higher channel not being 80 MHz",
           {{42, 0, 80, WIFI_PHY_BAND_5GHZ}, {102, 0, 80, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           true);

    RunOne("invalid 80+80 MHz (non-contiguous) OFDM channel lower channel not being 80 MHz",
           {{36, 0, 20, WIFI_PHY_BAND_5GHZ}, {106, 0, 80, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           true);

    RunOne("invalid 80+80 MHz (non-contiguous) OFDM channel with both segments configured on the "
           "same channel",
           {{42, 0, 80, WIFI_PHY_BAND_5GHZ}, {42, 0, 80, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           true);

    RunOne("invalid 80+80 MHz (non-contiguous) OFDM channel with segments configured to be "
           "contiguous (lower before higher)",
           {{42, 0, 80, WIFI_PHY_BAND_5GHZ}, {58, 0, 80, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           true);

    RunOne("invalid 80+80 MHz (non-contiguous) OFDM channel with segments configured to be "
           "contiguous (higher before lower)",
           {{58, 0, 80, WIFI_PHY_BAND_5GHZ}, {42, 0, 80, WIFI_PHY_BAND_5GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           true);

    RunOne("invalid 80+80 MHz (non-contiguous) OFDM channel with each segments configured on a "
           "different band",
           {{42, 0, 80, WIFI_PHY_BAND_5GHZ}, {215, 0, 80, WIFI_PHY_BAND_6GHZ}},
           WIFI_STANDARD_UNSPECIFIED,
           WIFI_PHY_BAND_UNSPECIFIED,
           true);
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Test the operating channel functions for 80+80MHz.
 */
class WifiPhyChannel80Plus80Test : public TestCase
{
  public:
    /**
     * Constructor
     */
    WifiPhyChannel80Plus80Test();
    ~WifiPhyChannel80Plus80Test() override = default;

  private:
    void DoRun() override;

    /**
     * Create a dummy PSDU whose payload is 1000 bytes
     * \return a dummy PSDU whose payload is 1000 bytes
     */
    Ptr<WifiPsdu> CreateDummyPsdu();

    /**
     * Create a HE PPDU
     * \param bandwidth the bandwidth used for the transmission the PPDU in MHz
     * \param channel the operating channel of the PHY used for the transmission
     * \return a HE PPDU
     */
    Ptr<HePpdu> CreateDummyHePpdu(ChannelWidthMhz bandwidth,
                                  const WifiPhyOperatingChannel& channel);

    WifiPhyOperatingChannel m_channel; //!< operating channel
};

WifiPhyChannel80Plus80Test::WifiPhyChannel80Plus80Test()
    : TestCase("Check operating channel functions for 80+80MHz")
{
}

Ptr<WifiPsdu>
WifiPhyChannel80Plus80Test::CreateDummyPsdu()
{
    Ptr<Packet> pkt = Create<Packet>(1000);
    WifiMacHeader hdr;
    hdr.SetType(WIFI_MAC_QOSDATA);
    hdr.SetQosTid(0);
    return Create<WifiPsdu>(pkt, hdr);
}

Ptr<HePpdu>
WifiPhyChannel80Plus80Test::CreateDummyHePpdu(ChannelWidthMhz bandwidth,
                                              const WifiPhyOperatingChannel& channel)
{
    WifiTxVector txVector =
        WifiTxVector(HePhy::GetHeMcs0(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0, bandwidth, false);
    Ptr<WifiPsdu> psdu = CreateDummyPsdu();
    return Create<HePpdu>(psdu, txVector, channel, MicroSeconds(100), 0);
}

void
WifiPhyChannel80Plus80Test::DoRun()
{
    // P20 is in first segment and segments are provided in increasing frequency order
    {
        m_channel.Set({{42, 0, 80, WIFI_PHY_BAND_5GHZ}, {106, 0, 80, WIFI_PHY_BAND_5GHZ}},
                      WIFI_STANDARD_UNSPECIFIED);
        m_channel.SetPrimary20Index(3);

        const auto indexPrimary160Mhz = m_channel.GetPrimaryChannelIndex(160);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary160Mhz, 0, "Primary 160 MHz channel shall have index 0");
        const auto indexPrimary80Mhz = m_channel.GetPrimaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary80Mhz, 0, "Primary 80 MHz channel shall have index 0");
        const auto indexPrimary40Mhz = m_channel.GetPrimaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary40Mhz, 1, "Primary 40 MHz channel shall have index 1");
        const auto indexPrimary20Mhz = m_channel.GetPrimaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary20Mhz, 3, "Primary 20 MHz channel shall have index 3");

        const auto indexSecondary80Mhz = m_channel.GetSecondaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary80Mhz,
                              1,
                              "Secondary 80 MHz channel shall have index 1");
        const auto indexSecondary40Mhz = m_channel.GetSecondaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary40Mhz,
                              0,
                              "Secondary 40 MHz channel shall have index 0");
        const auto indexSecondary20Mhz = m_channel.GetSecondaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary20Mhz,
                              2,
                              "Secondary 20 MHz channel shall have index 2");

        const auto primary80MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzCenterFrequency,
                              5210,
                              "Primary 80 MHz channel center frequency shall be 5210 MHz");
        const auto primary40MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzCenterFrequency,
                              5230,
                              "Primary 40 MHz channel center frequency shall be 5230 MHz");
        const auto primary20MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzCenterFrequency,
                              5240,
                              "Primary 20 MHz channel center frequency shall be 5240 MHz");

        const auto secondary80MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(secondary80MhzCenterFrequency,
                              5530,
                              "Secondary 80 MHz channel center frequency shall be 5530 MHz");
        const auto secondary40MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(secondary40MhzCenterFrequency,
                              5190,
                              "Secondary 40 MHz channel center frequency shall be 5190 MHz");
        const auto secondary20MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(secondary20MhzCenterFrequency,
                              5220,
                              "Secondary 20 MHz channel center frequency shall be 5220 MHz");

        const auto primary80MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(80, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzChannelNumber,
                              42,
                              "Primary 80 MHz channel number shall be 42");
        const auto primary40MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(40, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzChannelNumber,
                              46,
                              "Primary 40 MHz channel number shall be 46");
        const auto primary20MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(20, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzChannelNumber,
                              48,
                              "Primary 20 MHz channel number shall be 48");

        auto ppdu160MHz = CreateDummyHePpdu(160, m_channel);
        auto txCenterFreqs160MHz = ppdu160MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.size(), 2, "2 segments are covered by 160 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.front(),
                              5210,
                              "Center frequency of first segment shall be 5210 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.back(),
                              5530,
                              "Center frequency of second segment shall be 5530 MHz");
        auto ppdu80MHz = CreateDummyHePpdu(80, m_channel);
        auto txCenterFreqs80MHz = ppdu80MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.size(), 1, "1 segment is covered by 80 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.front(),
                              5210,
                              "Center frequency for 80 MHz shall be 5210 MHz");
        auto ppdu40MHz = CreateDummyHePpdu(40, m_channel);
        auto txCenterFreqs40MHz = ppdu40MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.size(), 1, "1 segment is covered by 40 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.front(),
                              5230,
                              "Center frequency for 40 MHz shall be 5230 MHz");
        auto ppdu20MHz = CreateDummyHePpdu(20, m_channel);
        auto txCenterFreqs20MHz = ppdu20MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.size(), 1, "1 segment is covered by 20 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.front(),
                              5240,
                              "Center frequency for 20 MHz shall be 5240 MHz");
    }

    // P20 is in second segment and segments are provided in increasing frequency order
    {
        m_channel.Set({{42, 0, 80, WIFI_PHY_BAND_5GHZ}, {106, 0, 80, WIFI_PHY_BAND_5GHZ}},
                      WIFI_STANDARD_UNSPECIFIED);
        m_channel.SetPrimary20Index(4);

        const auto indexPrimary160Mhz = m_channel.GetPrimaryChannelIndex(160);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary160Mhz, 0, "Primary 160 MHz channel shall have index 0");
        const auto indexPrimary80Mhz = m_channel.GetPrimaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary80Mhz, 1, "Primary 80 MHz channel shall have index 1");
        const auto indexPrimary40Mhz = m_channel.GetPrimaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary40Mhz, 2, "Primary 40 MHz channel shall have index 2");
        const auto indexPrimary20Mhz = m_channel.GetPrimaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary20Mhz, 4, "Primary 20 MHz channel shall have index 4");

        const auto indexSecondary80Mhz = m_channel.GetSecondaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary80Mhz,
                              0,
                              "Secondary 80 MHz channel shall have index 0");
        const auto indexSecondary40Mhz = m_channel.GetSecondaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary40Mhz,
                              3,
                              "Secondary 40 MHz channel shall have index 3");
        const auto indexSecondary20Mhz = m_channel.GetSecondaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary20Mhz,
                              5,
                              "Secondary 20 MHz channel shall have index 5");

        const auto primary80MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzCenterFrequency,
                              5530,
                              "Primary 80 MHz channel center frequency shall be 5530 MHz");
        const auto primary40MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzCenterFrequency,
                              5510,
                              "Primary 40 MHz channel center frequency shall be 5510 MHz");
        const auto primary20MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzCenterFrequency,
                              5500,
                              "Primary 20 MHz channel center frequency shall be 5500 MHz");

        const auto secondary80MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(secondary80MhzCenterFrequency,
                              5210,
                              "Secondary 80 MHz channel center frequency shall be 5210 MHz");
        const auto secondary40MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(secondary40MhzCenterFrequency,
                              5550,
                              "Secondary 40 MHz channel center frequency shall be 5550 MHz");
        const auto secondary20MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(secondary20MhzCenterFrequency,
                              5520,
                              "Secondary 20 MHz channel center frequency shall be 5520 MHz");

        const auto primary80MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(80, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzChannelNumber,
                              106,
                              "Primary 80 MHz channel number shall be 106");
        const auto primary40MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(40, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzChannelNumber,
                              102,
                              "Primary 40 MHz channel number shall be 102");
        const auto primary20MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(20, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzChannelNumber,
                              100,
                              "Primary 20 MHz channel number shall be 100");

        auto ppdu160MHz = CreateDummyHePpdu(160, m_channel);
        auto txCenterFreqs160MHz = ppdu160MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.size(), 2, "2 segments are covered by 160 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.front(),
                              5530,
                              "Center frequency of first segment shall be 5530 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.back(),
                              5210,
                              "Center frequency of second segment shall be 5210 MHz");
        auto ppdu80MHz = CreateDummyHePpdu(80, m_channel);
        auto txCenterFreqs80MHz = ppdu80MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.size(), 1, "1 segment is covered by 80 MHz");
        NS_ASSERT(txCenterFreqs80MHz.front() == 5530);
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.front(),
                              5530,
                              "Center frequency for 80 MHz shall be 5530 MHz");
        auto ppdu40MHz = CreateDummyHePpdu(40, m_channel);
        auto txCenterFreqs40MHz = ppdu40MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.size(), 1, "1 segment is covered by 40 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.front(),
                              5510,
                              "Center frequency for 40 MHz shall be 5510 MHz");
        auto ppdu20MHz = CreateDummyHePpdu(20, m_channel);
        auto txCenterFreqs20MHz = ppdu20MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.size(), 1, "1 segment is covered by 20 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.front(),
                              5500,
                              "Center frequency for 20 MHz shall be 5500 MHz");
    }

    // P20 is in first segment and segments are provided in decreasing frequency order
    {
        m_channel.Set({{106, 0, 80, WIFI_PHY_BAND_5GHZ}, {42, 0, 80, WIFI_PHY_BAND_5GHZ}},
                      WIFI_STANDARD_UNSPECIFIED);
        m_channel.SetPrimary20Index(3);

        const auto indexPrimary160Mhz = m_channel.GetPrimaryChannelIndex(160);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary160Mhz, 0, "Primary 160 MHz channel shall have index 0");
        const auto indexPrimary80Mhz = m_channel.GetPrimaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary80Mhz, 0, "Primary 80 MHz channel shall have index 0");
        const auto indexPrimary40Mhz = m_channel.GetPrimaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary40Mhz, 1, "Primary 40 MHz channel shall have index 1");
        const auto indexPrimary20Mhz = m_channel.GetPrimaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary20Mhz, 3, "Primary 20 MHz channel shall have index 3");

        const auto indexSecondary80Mhz = m_channel.GetSecondaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary80Mhz,
                              1,
                              "Secondary 80 MHz channel shall have index 1");
        const auto indexSecondary40Mhz = m_channel.GetSecondaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary40Mhz,
                              0,
                              "Secondary 40 MHz channel shall have index 0");
        const auto indexSecondary20Mhz = m_channel.GetSecondaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary20Mhz,
                              2,
                              "Secondary 20 MHz channel shall have index 2");

        const auto primary80MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzCenterFrequency,
                              5210,
                              "Primary 80 MHz channel center frequency shall be 5210 MHz");
        const auto primary40MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzCenterFrequency,
                              5230,
                              "Primary 40 MHz channel center frequency shall be 5230 MHz");
        const auto primary20MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzCenterFrequency,
                              5240,
                              "Primary 20 MHz channel center frequency shall be 5240 MHz");

        const auto secondary80MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(secondary80MhzCenterFrequency,
                              5530,
                              "Secondary 80 MHz channel center frequency shall be 5530 MHz");
        const auto secondary40MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(secondary40MhzCenterFrequency,
                              5190,
                              "Secondary 40 MHz channel center frequency shall be 5190 MHz");
        const auto secondary20MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(secondary20MhzCenterFrequency,
                              5220,
                              "Secondary 20 MHz channel center frequency shall be 5220 MHz");

        const auto primary80MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(80, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzChannelNumber,
                              42,
                              "Primary 80 MHz channel number shall be 42");
        const auto primary40MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(40, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzChannelNumber,
                              46,
                              "Primary 40 MHz channel number shall be 46");
        const auto primary20MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(20, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzChannelNumber,
                              48,
                              "Primary 20 MHz channel number shall be 48");

        auto ppdu160MHz = CreateDummyHePpdu(160, m_channel);
        auto txCenterFreqs160MHz = ppdu160MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.size(), 2, "2 segments are covered by 160 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.front(),
                              5210,
                              "Center frequency of first segment shall be 5210 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.back(),
                              5530,
                              "Center frequency of second segment shall be 5530 MHz");
        auto ppdu80MHz = CreateDummyHePpdu(80, m_channel);
        auto txCenterFreqs80MHz = ppdu80MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.size(), 1, "1 segment is covered by 80 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.front(),
                              5210,
                              "Center frequency for 80 MHz shall be 5210 MHz");
        auto ppdu40MHz = CreateDummyHePpdu(40, m_channel);
        auto txCenterFreqs40MHz = ppdu40MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.size(), 1, "1 segment is covered by 40 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.front(),
                              5230,
                              "Center frequency for 40 MHz shall be 5230 MHz");
        auto ppdu20MHz = CreateDummyHePpdu(20, m_channel);
        auto txCenterFreqs20MHz = ppdu20MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.size(), 1, "1 segment is covered by 20 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.front(),
                              5240,
                              "Center frequency for 20 MHz shall be 5240 MHz");
    }

    // P20 is in second segment and segments are provided in decreasing frequency order
    {
        m_channel.Set({{106, 0, 80, WIFI_PHY_BAND_5GHZ}, {42, 0, 80, WIFI_PHY_BAND_5GHZ}},
                      WIFI_STANDARD_UNSPECIFIED);
        m_channel.SetPrimary20Index(4);

        const auto indexPrimary160Mhz = m_channel.GetPrimaryChannelIndex(160);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary160Mhz, 0, "Primary 160 MHz channel shall have index 0");
        const auto indexPrimary80Mhz = m_channel.GetPrimaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary80Mhz, 1, "Primary 80 MHz channel shall have index 1");
        const auto indexPrimary40Mhz = m_channel.GetPrimaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary40Mhz, 2, "Primary 40 MHz channel shall have index 2");
        const auto indexPrimary20Mhz = m_channel.GetPrimaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexPrimary20Mhz, 4, "Primary 20 MHz channel shall have index 4");

        const auto indexSecondary80Mhz = m_channel.GetSecondaryChannelIndex(80);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary80Mhz,
                              0,
                              "Secondary 80 MHz channel shall have index 0");
        const auto indexSecondary40Mhz = m_channel.GetSecondaryChannelIndex(40);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary40Mhz,
                              3,
                              "Secondary 40 MHz channel shall have index 3");
        const auto indexSecondary20Mhz = m_channel.GetSecondaryChannelIndex(20);
        NS_TEST_ASSERT_MSG_EQ(indexSecondary20Mhz,
                              5,
                              "Secondary 20 MHz channel shall have index 5");

        const auto primary80MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzCenterFrequency,
                              5530,
                              "Primary 80 MHz channel center frequency shall be 5530 MHz");
        const auto primary40MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzCenterFrequency,
                              5510,
                              "Primary 40 MHz channel center frequency shall be 5510 MHz");
        const auto primary20MhzCenterFrequency = m_channel.GetPrimaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzCenterFrequency,
                              5500,
                              "Primary 20 MHz channel center frequency shall be 5500 MHz");

        const auto secondary80MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(80);
        NS_TEST_ASSERT_MSG_EQ(secondary80MhzCenterFrequency,
                              5210,
                              "Secondary 80 MHz channel center frequency shall be 5210 MHz");
        const auto secondary40MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(40);
        NS_TEST_ASSERT_MSG_EQ(secondary40MhzCenterFrequency,
                              5550,
                              "Secondary 40 MHz channel center frequency shall be 5550 MHz");
        const auto secondary20MhzCenterFrequency = m_channel.GetSecondaryChannelCenterFrequency(20);
        NS_TEST_ASSERT_MSG_EQ(secondary20MhzCenterFrequency,
                              5520,
                              "Secondary 20 MHz channel center frequency shall be 5520 MHz");

        const auto primary80MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(80, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary80MhzChannelNumber,
                              106,
                              "Primary 80 MHz channel number shall be 106");
        const auto primary40MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(40, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary40MhzChannelNumber,
                              102,
                              "Primary 40 MHz channel number shall be 102");
        const auto primary20MhzChannelNumber =
            m_channel.GetPrimaryChannelNumber(20, WIFI_STANDARD_80211ax);
        NS_TEST_ASSERT_MSG_EQ(primary20MhzChannelNumber,
                              100,
                              "Primary 20 MHz channel number shall be 100");

        auto ppdu160MHz = CreateDummyHePpdu(160, m_channel);
        auto txCenterFreqs160MHz = ppdu160MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.size(), 2, "2 segments are covered by 160 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.front(),
                              5530,
                              "Center frequency of first segment shall be 5530 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs160MHz.back(),
                              5210,
                              "Center frequency of second segment shall be 5210 MHz");
        auto ppdu80MHz = CreateDummyHePpdu(80, m_channel);
        auto txCenterFreqs80MHz = ppdu80MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.size(), 1, "1 segment is covered by 80 MHz");
        NS_ASSERT(txCenterFreqs80MHz.front() == 5530);
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs80MHz.front(),
                              5530,
                              "Center frequency for 80 MHz shall be 5530 MHz");
        auto ppdu40MHz = CreateDummyHePpdu(40, m_channel);
        auto txCenterFreqs40MHz = ppdu40MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.size(), 1, "1 segment is covered by 40 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs40MHz.front(),
                              5510,
                              "Center frequency for 40 MHz shall be 5510 MHz");
        auto ppdu20MHz = CreateDummyHePpdu(20, m_channel);
        auto txCenterFreqs20MHz = ppdu20MHz->GetTxCenterFreqs();
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.size(), 1, "1 segment is covered by 20 MHz");
        NS_TEST_ASSERT_MSG_EQ(txCenterFreqs20MHz.front(),
                              5500,
                              "Center frequency for 20 MHz shall be 5500 MHz");
    }

    Simulator::Destroy();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief wifi operating channel test suite
 */
class WifiOperatingChannelTestSuite : public TestSuite
{
  public:
    WifiOperatingChannelTestSuite();
};

WifiOperatingChannelTestSuite::WifiOperatingChannelTestSuite()
    : TestSuite("wifi-operating-channel", UNIT)
{
    AddTestCase(new SetWifiOperatingChannelTest(), TestCase::QUICK);
    AddTestCase(new WifiPhyChannel80Plus80Test(), TestCase::QUICK);
}

static WifiOperatingChannelTestSuite g_wifiOperatingChannelTestSuite; ///< the test suite
