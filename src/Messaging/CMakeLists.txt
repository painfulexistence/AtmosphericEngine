set(SOURCES
    src/Messaging/Messagable.cpp
    src/Messaging/MessageBus.cpp
    src/Messaging/Message.cpp
)
target_sources(AtmosphericEngine PRIVATE ${SOURCES})