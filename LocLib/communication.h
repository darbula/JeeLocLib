#ifndef DisAlgProtocol_h
#define DisAlgProtocol_h

class DisAlgProtocol {
  private:
    uint8_t ID;
  public:
    uint8_t RESEND_DELAY_BASE;
    uint8_t RESEND_COUNT_BASE;
    DisAlgProtocol (uint8_t node_id);
    DisAlgProtocol (uint8_t node_id, uint8_t RESEND_DELAY_BASE,
                    uint8_t RESEND_COUNT_BASE);
    void send(uint8_t type, uint8_t destination, void* message, uint8_t len);
    uint8_t send_req_ack(uint8_t type, uint8_t destination, void* message,
                         uint8_t len);
    uint8_t receive(void* message);
};

#endif // DisAlgProtocol_h