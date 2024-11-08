#include <cstring>

#include <mesh/message.h>

namespace mesh {

Buffer Serialize::operator()(const Message& message) const noexcept {
    Buffer result{};
    using Char_type = Buffer::value_type;

    /* Reserve space for the header */
    result.push_back(static_cast<Char_type>(message.m_header.m_type));

    /* Message ID */
    auto id_bytes = reinterpret_cast<const Char_type*>(&message.m_header.m_message_id);
    result.insert(result.end(), id_bytes, id_bytes + sizeof(message.m_header.m_message_id));

    /* Source ID length and data */
    const auto &peer_id = message.m_header.m_source_id;
    auto id_length_bytes = static_cast<uint16_t>(peer_id.length());
    result.insert(result.end(), id_length_bytes, id_length_bytes + sizeof(id_length_bytes));
    result.insert(result.end(), peer_id.begin(), peer_id.end());

    /* TTL */
    auto ttl_bytes = reinterpret_cast<const Char_type*>(&message.m_header.m_ttl);
    result.insert(result.end(), ttl_bytes, ttl_bytes + sizeof(message.m_header.m_ttl));

    /*  Payload */
    result.insert(result.end(), message.m_payload.begin(), message.m_payload.end());

    return result;
}

Message Deserialize::operator()(const Buffer& data) const noexcept {
    Message msg;
    size_t pos{};

    /* Message type */
    msg.m_header.m_type = static_cast<Message_type>(data[pos++]);

    /* Message ID */
    msg.m_header.m_message_id = *reinterpret_cast<const decltype(msg.m_header.m_message_id)*>(&data[pos]);
    pos += sizeof(decltype(msg.m_header.m_message_id));

    /* Source ID */
    auto id_length = *reinterpret_cast<const uint16_t*>(&data[pos]);
    pos += sizeof(uint16_t);
    msg.m_header.m_source_id.assign(data.begin() + pos, data.begin() + pos + id_length);
    pos += id_length;

    /* TTL */
    msg.m_header.m_ttl = *reinterpret_cast<const decltype(msg.m_header.m_ttl)*>(&data[pos]);
    pos += sizeof(decltype(msg.m_header.m_ttl));

    /* Payload */
    msg.m_payload.assign(data.begin() + pos, data.end());

    return msg;
}

} // namespace mesh
