package ApiCommonTypes

type NewSessionTokenRequest struct {
	RefreshToken string `json:"RefreshToken"`
}

type NewSessionTokenResponse struct {
	SessionToken string `json:"SessionToken"`
}

type LoginRequest struct {
	Email    string `json:"email"`
	Password string `json:"password"`
}

type LoginResponse struct {
	RefreshToken string `json:"RefreshToken"`
	SessionToken string `json:"SessionToken"`
	UserID       string `json:"UserID"`
}

type RegisterRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
	Email    string `json:"email"`
}

type RegisterResponse struct {
	RefreshToken string `json:"RefreshToken"`
	SessionToken string `json:"SessionToken"`
	UserID       string `json:"UserID"`
}

type UpdatePasswordRequest struct {
	OldPassword  string `json:"oldPassword"`
	RefreshToken string `json:"RefreshToken"`
	NewPassword  string `json:"newPassword"`
}

type UpdatePasswordResponse struct {
	// Empty response
}
type UpdateUsernameRequest struct {
	OldPassword  string `json:"oldPassword"`
	RefreshToken string `json:"RefreshToken"`
	NewUsername  string `json:"newPassword"`
}

type UpdateUsernameResponse struct {
	// Empty response
}

type DeleteAccountRequest struct {
	Password     string `json:"password"`
	RefreshToken string `json:"RefreshToken"`
}

type DeleteAccountResponse struct {
	// Empty response
}
