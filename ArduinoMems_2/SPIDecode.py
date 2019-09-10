"""
Connect logic analyzer to SPI output, set SPI decoder to defaults except for
"24 bits per transfer". Paste the decoded transfer value into here
to decode into commands for AD5664.
"""

import matplotlib.pyplot as plt
import numpy as np


def Decode(spi_Dec_String):
    # print(hex(spi_Dec_String))

    spi_Bits = bin(spi_Dec_String)[2:].rjust(24, "0")

    # dont_Care = spi_Bits[0:2]
    command = spi_Bits[2:5]
    address = spi_Bits[5:8]
    data = spi_Bits[8:]

    # print(command, address, data)
    # print(f"Data bits={int(data,2)}")

    return command, address, int(data, 2)


all_SPI_Data = np.loadtxt("spi.csv",
                          delimiter="'",
                          skiprows=1,
                          usecols=1,
                          dtype=np.int)

rows = all_SPI_Data.shape[0] // 5

spi_Decoded = np.array(
        [Decode(spi_Bits)[2] for spi_Bits in all_SPI_Data],
        dtype=np.int)[:rows*5].reshape((-1, 5))

xy_Vals = [(x[0], x[3]) for x in spi_Decoded[:,:4]]

plt.plot(xy_Vals)
plt.show()

print(len(xy_Vals), "xy vals")