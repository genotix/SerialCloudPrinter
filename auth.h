/*
 * AUTH
 * 
 * Manages the authentication keys for the specific device
 * TODO: Move these keys to the LittleFS filesystem
 * 
 */
 
// Perhaps this will help to read certain certificates from disk
// https://stackoverflow.com/questions/61987375/arduino-converting-value-from-file-read-to-const-char

/* Define the Security authentication for the device; for AWS IoT core "things":
    A certificate for this thing 9be8959693.cert.pem        <--  my_cert[]
    A public key  9be8959693.public.key
    A private key 9be8959693.private.key                    <--- my_key[]

 */

// AC67B2FA6FD4

const char my_cert[] PROGMEM = R"==(-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUcchvKQzL220JxIWF3rA3Pf2wq0QwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIxMDQwNzE0MjAx
NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAO8KG6OZxW6IpZeGdj6e
ZYjnlsWgx1kf/jABTHK4+MvtGAJbf3MR+qo4WZbsxhzyybdYrQLsF1EkNA+59TpM
FYeaauD7lMZFZQ1mPerGh5ShHAKiaxy5Yni2aV31oZwv6OM09uuicYYIg7RSu1ik
wZtnpCpjgmPR2PgpwQjRnEQoIwRc1F1sNyX0QuWrmW47Yx/FEFs/MT0z/Pnz09LT
QFFjFghV0SZwNPgPKJDBmoKDpT05608DmZ3VC15sjHFvOdJAmXrxKRNUZ+J6g/h8
6eef3tHOAsgeIO1Cxpt2boF5Dw6B0ak8OK+MBF+l7BmJKDwNU7aIXmDmsdjB/TkZ
P00CAwEAAaNgMF4wHwYDVR0jBBgwFoAUFDqokeJ4foqBfRL7mTJutn2LauEwHQYD
VR0OBBYEFCiw3uNbcvtX65FkLIDtNRbijJFUMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQC+i2t78rM85aZ/ouuvw8Qxs3Ye
GkOBmeS7iN+o3Sw/mHGP/KmnqFKSmg98PQq7ywFGB24vNb9ndifH2s5nQmBt1xym
OUR0/qLdKSlxd1QjqrI44Da6JpCppJdVrye/75ZqYgLG373dFUvNhW6MwmmGX9x8
pbAfwnjqiSNDNkGkOpjGsZKb+Umd8HgyN/SXpU9IjWo0oNyvGIw7hJrBWdYB7OBq
ZV8koQYpcA8+Hd9oBUflHCP++wXEwtQ0QEkZiwyYtF9X3vCpkkbbH14ig7BMMgRx
vlKA5I+sAZy/vFJynS/D4+HXULa4lOVB/SfqNemvwKE/mFj/hZCyB7AuD9S4
-----END CERTIFICATE-----)==";

const char my_key[] PROGMEM = R"==(-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA7wobo5nFboill4Z2Pp5liOeWxaDHWR/+MAFMcrj4y+0YAlt/
cxH6qjhZluzGHPLJt1itAuwXUSQ0D7n1OkwVh5pq4PuUxkVlDWY96saHlKEcAqJr
HLlieLZpXfWhnC/o4zT266JxhgiDtFK7WKTBm2ekKmOCY9HY+CnBCNGcRCgjBFzU
XWw3JfRC5auZbjtjH8UQWz8xPTP8+fPT0tNAUWMWCFXRJnA0+A8okMGagoOlPTnr
TwOZndULXmyMcW850kCZevEpE1Rn4nqD+Hzp55/e0c4CyB4g7ULGm3ZugXkPDoHR
qTw4r4wEX6XsGYkoPA1TtoheYOax2MH9ORk/TQIDAQABAoIBAGmWqHDS2vAhciwF
nDO62hToX6Q6ifQnequP10oRxOyndWSNMDYPKg+IxqIxQq3E3S+c/wd8bCxOdqS/
usAenaABqNZqquQOAT4y/IS5X2ha5jmPrwrIJOQ3h0GW3+VTEGdUnQGnBshy+GJt
7Q2R8WeJF24V+KHJLzMnlnojxQghbAWxdoL0YBBklT2YnXoi9FJxcIb+MWB22lP5
ulHJN4lOnNXHVsJsuBB1ZKRqd1DLoxKtUFsTiXYU/dB50iHR206Kaksm7U0XaPA5
qgBqrgUNakRXzM9/Di7IMd04MTRr2GXtJEddXp+pKSurPo2c5GfXb4RTMZt6PYt8
mZt1Pq0CgYEA/N02l9ptp2w6IH6I0j1wKGt9LCgokQRoDPkab9W0F7sgIzdDerBF
mLeRkOis0W1u0rKoqH35glJT2ktOI8OdJbhtUEziOD4M7X5iYcyuxXCfOjcaVIcA
YpSVK6lrlq99F+FOTS/eh5pXfXqR6waOS3/hnvBEFMM3rf8+WY0abJcCgYEA8gEB
Lotwyb9T8kJeC016KQOxvniNv543b8C331rWJ9SmBphnPGcnyPqbqsJqQ//pSfBE
2N9QLUzPNzLnfNXu5a4DIzYYRmz6GbHLKSpSib0kdrXnHGtnvunupAzc88beM30h
LMx6Fk9ZyNk0/LT274v32wy4+3INjoxAHMBoG7sCgYEA4COF/NzVTg6VB7ChvxpO
jMWa0sVWEBWD1dwJHxCi7lYkipK18c7GIjS1h2l3prwF/CX5ckEYJfir81fFD9aZ
OZJc80zzST9XXj0lH+O7F7BYs9WcNUjlgnyKLYWjpJIxpDVGnwzLiA+7dbJsHZRW
FX5pk+UNtAnHFSinL+HvhkkCgYBv7lcFmCkKpxvEqT2mLKeb4MGNP2UQg7VQUuq4
u+LheVFxK3xUS8HzCzaEvHc6DFtyf1bYmDEVQrl05m57JtfgE6QP3S1NJYpn5/v3
hkO2wWzkGy5Rzrulab2e0vQ/LHQJjI6Tkd/GAd2dMBl56JluImb5QeEtK4tpqyTc
J4H/kQKBgQDHtomYTULxTSNKEw+UCbU0vDObFBSW1NMtHbqpMPznfleIOzgVob4g
gmuNPTMJzLmEPf/ehKZX7lFtU/lcrm26ZiWmMsngdEW6q5upmxhYtXYRbjqt41Ix
XsudB42HbXpEGcT8++xNDkwaugn2pbuth6xmMAO69zHiSqzmr5KNlQ==
-----END RSA PRIVATE KEY-----)==";
