# About
This repo contains a proof-of-concept/prototype/WIP session-less LAN discovery and matchmaking plugin written for an internal project. It's up here solely for sharing purposes given the scarcity of documentation and samples covering the `IMessageBus` and friends interface, and is __definitely not suitable for general re-use__ since it's very project-specific, lacking any tests, and is probably full of booby traps in the shape of hastily-written noob c++ code.

Below is a description of my specific use case for using the message bus (as opposed to th Session interface) [my specific use case](#good-for), and more [resources](#more-info).   

## Good for
- Instant device discovery and message broadcasting on local network
- No time delays caused by session broadcasting/discovering/registration/joining
- Connection always established regardless of Client/Server travel

## More Info
- Mclaren Car Configurator [twitch stream](https://www.youtube.com/watch?reload=9&v=tjvKsEcbHk0) where the MessageBus is talked about and demoed
- [Slides](https://www.slideshare.net/GerkeMaxPreussner/ue4-twitch-2016-0505-unreal-message-bus-overview) by Gerke Max Preussner
- [Code sample](https://github.com/BhaaLseN/UnrealMessageBusDemo) by BhhaaLseN
- For debugging, enable the built-in MessagingDebugger plugin and look under Window->Developer Tools for [this](https://gmpreussner.com/portfolio/message-bus) little gem
