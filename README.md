## About
This is a prototype LAN discovery/matchmaking plugin written for an internal UE4 project.

Below is a description of my specific use case for using the message bus (as opposed to the Session interface) [my specific use case](#good-for), and more [resources](#more-info).   

## Good for
- Instant device discovery and message broadcasting on local network
- No time delays caused by session broadcasting/discovering/registration/joining
- Connection always established regardless of Client/Server travel

## More Info
- Mclaren Car Configurator [twitch stream](https://www.youtube.com/watch?reload=9&v=tjvKsEcbHk0) where the MessageBus is talked about and demoed
- [Slides](https://www.slideshare.net/GerkeMaxPreussner/ue4-twitch-2016-0505-unreal-message-bus-overview) by Gerke Max Preussner
- [Code sample](https://github.com/BhaaLseN/UnrealMessageBusDemo) by BhhaaLseN
- For debugging, enable the built-in MessagingDebugger plugin and look under Window->Developer Tools for [this](https://gmpreussner.com/portfolio/message-bus) little gem
