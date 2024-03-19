# Side Channel Attack on GIFT COFB (Attack on Lightweight Cryptography)

GIFT-COFB is an Authenticated Encryption with Associated Data (AEAD) scheme, based on GIFT lightweight block cipher and, COFB lightweight AEAD operating mode. Authenticated Encryption with Associated Data (AEAD) scheme provides both conﬁdentiality and authenticity.

Here we try to perform Power Side Channel Attacks on the hardware running GIFT-COFB. Power traces of ChipWhisperer Nano running software implementation of GIFT-COFB are collected and Correlation Power Analysis and Difference-of-Mean attacks are performed on the hardware. We observe both fail to obtain the secret key. 

For further details refer to [video](https://github.com/GentlemanAMS/GIFT-COFB-SCA/blob/master/video.mp4) and [presentation](https://github.com/GentlemanAMS/GIFT-COFB-SCA/blob/master/presentation.pdf).
