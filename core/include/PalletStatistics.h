#pragma once

class PalletStatistics
{
private:

    int palletId;

    double usedVolume;

    double totalVolume;

    double utilization;

public:

    PalletStatistics();

    PalletStatistics(
        int palletId,
        double usedVolume,
        double totalVolume);

    int getPalletId() const;

    double getUsedVolume() const;

    double getTotalVolume() const;

    double getUtilization() const;

    void setUsedVolume(
        double usedVolume);

    void setTotalVolume(
        double totalVolume);

    void updateUtilization();
};