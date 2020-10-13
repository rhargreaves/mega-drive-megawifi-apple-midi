# Mega Drive MegaWiFi & Apple MIDI Network POC [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-megawifi-apple-midi.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-megawifi-apple-midi) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-megawifi-apple-midi?style=plastic)](https://github.com/rhargreaves/mega-drive-megawifi-apple-midi/releases)

Receiving & sending MIDI events via over MegaWiFi using the Apple MIDI Network protocol

## Support

* Apple MIDI session negotiation
* RTP-MIDI packets
  * Both long and short header form
  * SysEx commands contained in one single command & packet
  * 2-byte and 3-byte MIDI events

## Limitations

* RTP-MIDI: Phantom status packets are not supported (P = 1)
* RTP-MIDI: Delta timestamp is not permitted in first command (Z = 1)
* Apple MIDI: Mega Drive returns zero timestamp in time synchronisation

## Build

Docker:

```sh
./docker-make
```

Linux (requires `cmake` & [gendev](https://github.com/kubilus1/gendev)):

```sh
make
```

## Light-Reading

 * [RFC-6295](https://tools.ietf.org/html/rfc6295)
 * [Apple Network MIDI Protocol](https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html)
