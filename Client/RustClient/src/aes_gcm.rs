use aes_gcm::{
    Aes256Gcm,
    Key, // Or `Aes128Gcm`
    Nonce,
    aead::{Aead, AeadCore, KeyInit, OsRng},
};
pub struct Cryption_Env {
    data: Vec<u8>,
    key: Vec<u8>,
    iv: Vec<u8>,
    cypher: Vec<u8>,
    auth: Vec<u8>,
}

impl Cryption_Env {
    fn new() -> Cryption_Env {
        let key = Aes256Gcm::generate_key(OsRng);
        let key: &[u8; 32] = &[42; 32];
        let key: &Key<Aes256Gcm> = key.into();
        
    }
}
