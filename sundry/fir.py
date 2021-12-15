import numpy as np
from scipy import signal


numtaps = 29
f = 6/24
test_data_length = 128
snr_threshold = 75


def snr(data, axis=0, ddof=0):
    """
    Calculate the signal-to-noise ratio of the data.

    Parameters
    ----------
    data : array_like
        Input data that is to be measured.
    axis : int or None, optional
        Axis along which to calculate the standard deviation. Default is 0.
        If None, compute over the whole array `data`.
    ddof : int, optional
        Means Delta Degrees of Freedom. The divisor used in calculations
        is ``N - ddof``, where ``N`` represents the number of elements.
        By default `ddof` is zero.

    Returns
    -------
    s2n : float
        The signal-to-noise ratio (in dB).

    """
    data = np.asanyarray(data)
    m = np.mean(data, axis)
    sd = np.std(data, axis, ddof=ddof)
    return 20 * np.log10(m / sd)

param = signal.firwin(numtaps, f)

data = np.array([
1009.000000, 974.000000, 974.000000, 1009.000000, 1009.000000, 1014.000000, 986.000000, 986.000000, 1031.000000, 1031.000000, 1001.000000, 1001.000000, 1006.000000, 1006.000000, 1007.000000, 1007.000000, 1010.000000, 1010.000000, 1011.000000, 1011.000000, 1007.000000, 1006.000000, 1006.000000, 1006.000000, 1006.000000, 1010.000000, 1010.000000, 1008.000000, 1008.000000, 1006.000000, 1006.000000, 1006.000000, 1006.000000, 1009.000000, 1009.000000, 1009.000000, 1007.000000, 1007.000000, 1008.000000, 1008.000000, 1007.000000, 1007.000000, 1009.000000, 1009.000000, 1010.000000, 1010.000000, 1007.000000, 1007.000000, 1007.000000, 1006.000000, 1006.000000, 1007.000000, 1007.000000, 1008.000000, 1008.000000, 1009.000000, 1009.000000, 1014.000000, 1014.000000, 1003.000000, 1003.000000, 998.000000, 998.000000, 1011.000000, 1016.000000, 1016.000000, 1006.000000, 1006.000000, 1001.000000, 1001.000000, 1008.000000, 1008.000000, 1012.000000, 1012.000000, 1008.000000, 1008.000000, 1000.000000, 1008.000000, 1008.000000, 1014.000000, 1014.000000, 1009.000000, 1009.000000, 1003.000000, 1003.000000, 1005.000000, 1005.000000, 1013.000000, 1013.000000, 1009.000000, 1009.000000, 1002.000000, 1006.000000, 1006.000000, 1012.000000, 1012.000000, 1008.000000, 1008.000000, 1003.000000, 1003.000000, 1006.000000, 1006.000000, 1010.000000, 1010.000000, 1008.000000, 1008.000000, 1005.000000, 1006.000000, 1006.000000, 1008.000000, 1008.000000, 1007.000000, 1007.000000, 1007.000000, 1007.000000, 1009.000000, 1009.000000, 1010.000000, 1010.000000, 1007.000000, 1005.000000, 1005.000000, 1008.000000, 1008.000000, 1011.000000, 1011.000000, 1009.000000, 1009.000000
])

filtered = np.convolve(data, param)

print(filtered)
print("\n\n\n")
for i in range(test_data_length):
    print("{:.4f}" .format(filtered[i]), end="\n")
print("")
print(snr(filtered))

if snr(filtered) < snr_threshold:
    print("Success")
else:
    print("you r trash")