# Authentication Plugin Guide

## Overview

The openmediavault authentication system now supports extensible authentication mechanisms through a 2-step login flow that allows plugins to inject additional authentication challenges (e.g., TOTP MFA, WebAuthn, LDAP).

## Architecture

### 2-Step Login Flow

1. **Authenticate** (Step 1): Client sends username/password
   - Backend validates credentials
   - If challenge required, the in-progress login is stored server-side
     (bound to the PHP session cookie) and the challenge info is returned
   - If no challenge, returns full session data

2. **Verify** (Step 2): Client submits the challenge response
   - The in-progress login is identified by the session cookie, so no token
     needs to be passed
   - Backend verifies challenge
   - Initializes full session on success
   - Returns session data with permissions

### RPC Methods

#### `Session::authenticate(username, password)`

**Response:**
```php
{
  "status": "authenticated" | "challengeRequired",
  "username": "...",
  "sessionid": "...",        // only if status="authenticated"
  "permissions": {...},      // only if status="authenticated"
  "challenge": {             // only if status="challengeRequired"
    "kind": "totp",          // Exposed to client
    "redirecturl": "/path/to/challenge"  // Exposed to client
    // Note: Other fields like "label", "digits", "periodSeconds" are stored
    // server-side but NOT included in this response. They're available to
    // the plugin via challengedata in verifyChallenge().
  }
}
```

#### `Session::verify(challengeresponse)`

**Request:**
```php
{
  "challengeresponse": {/*challenge-specific data*/}
}
```

**Response:**
```php
{
  "authenticated": true,
  "username": "...",
  "permissions": {...},
  "sessionid": "..."
}
```

#### `UserMgmt::verifyChallenge(username, challengeresponse, challengedata)`

This is the extension point where plugins verify challenge responses.

### Challenge Contract

- `challenge` MUST be an associative array/object (not a numeric list).
- `challenge.kind` is required and must be a non-empty string.
- **Only `kind` and `redirecturl` are exposed to clients** in the `Session::authenticate` response.
- All other challenge fields are stored server-side in the PHP session and passed to `verifyChallenge()`.

**Client-exposed fields** (returned in `Session::authenticate` response):
```php
[
  'kind',        // Required: identifies the challenge type (e.g., 'totp', 'webauthn')
  'redirecturl'  // Optional: URL to redirect the user for challenge input
]
```

**Server-side fields** (available to plugin's `verifyChallenge` method via `challengedata` param):
```php
[
  'label',              // UI label for the challenge
  'digits',             // Number of digits for TOTP codes
  'periodSeconds',      // TOTP time period
  'maskedDestination',  // Masked phone/email for SMS/email challenges
  'retryAfterSeconds',  // Rate limit info
  'attemptId',          // Server-side attempt tracking ID
  'expiresAt',          // Challenge expiration timestamp
  // ... any other plugin-specific metadata
]
```

Plugins can include any fields in the challenge returned from `authUser()`. The `Session::authenticate` 
method will expose only `kind` and `redirecturl` to the client, while storing the full challenge 
server-side for later verification.

---

## Implementing an Auth Plugin

### Step 1: Create a Plugin RPC Service

Create a new RPC service class that extends `\OMV\Rpc\ServiceAbstract`:

```php
<?php
namespace Engined\Rpc;

class MyAuthMgmt extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return "MyAuthMgmt";
    }

    public function initialize()
    {
        // Register hooks into the authentication flow
        $this->registerMutatingMethod(
            "mutateAuthUser",     // Method that modifies authUser response
            "UserMgmt",           // Target service
            "authUser"            // Target method
        );
        $this->registerMutatingMethod(
            "verifyMfaChallenge",  // Method that verifies challenge
            "UserMgmt",
            "verifyChallenge"
        );
    }

    /**
     * Hook: Mutate the authUser response to add challenge.
     * 
     * This method is called AFTER UserMgmt::authUser() returns successfully.
     * You can modify the response to add a challenge if needed.
     * 
     * IMPORTANT: The Session::authenticate method will expose only 'kind' and
     * 'redirecturl' to the client. All other fields are stored server-side in
     * the PHP session and passed to verifyChallenge() via the 'challengedata'
     * parameter.
     * 
     * @param array $params Original request params (username, password)
     * @param array $context RPC context
     * @param array $result Original authUser result
     * @return array Modified result
     */
    final public function mutateAuthUser($params, $context, $result)
    {
        // Only add challenge for users that need MFA
        if (!$this->userRequiresMfa($result['username'])) {
            return $result;
        }

        // Add MFA challenge to response.
        // Only 'kind' and 'redirecturl' will be sent to the client.
        // All fields are stored server-side and available in verifyChallenge().
        $result['challenge'] = [
            "kind" => "totp",                      // Exposed to client
            "redirecturl" => "/totp",              // Exposed to client
            "label" => "Enter the code from the Authenticator app",  // Server-side only
            "digits" => 6,                         // Server-side only
            "periodSeconds" => 30,                 // Server-side only
            "attemptId" => $this->generateAttemptId()  // Server-side only for tracking
        ];

        return $result;
    }

    /**
     * Hook: Verify the MFA challenge.
     * 
     * This method is called when Session::verify() is invoked with a challenge response.
     * 
     * The 'challengedata' parameter contains the FULL challenge object that was
     * returned from mutateAuthUser(), including server-side-only fields that were
     * not exposed to the client.
     * 
     * @param array $params Contains:
     *   - username: The authenticated user
     *   - challengeresponse: The user's challenge response (from client)
     *   - challengedata: The full original challenge (including server-side fields)
     * @param array $context RPC context
     * @param array $result Original verifyChallenge result (verified=false by default)
     * @return array Modified result with verified status
     */
    final public function verifyMfaChallenge($params, $context, $result)
    {
        $username = $params['username'];
        $challengeResponse = $params['challengeresponse'];
        $challengeData = $params['challengedata'];
        
        // Access server-side challenge fields (not exposed to client)
        $attemptId = $challengeData['attemptId'] ?? null;
        
        // Verify MFA code
        if (!$this->verifyTotpCode($username, $challengeResponse['code'])) {
            $result['verified'] = false;
            syslog(LOG_WARNING, "TOTP verification failed for user: $username (attempt: $attemptId)");
            return $result;
        }

        // TOTP code verified successfully
        $result['verified'] = true;
        syslog(LOG_NOTICE, "TOTP verification succeeded for user: $username");
        
        return $result;
    }

    private function userRequiresMfa($username)
    {
        // Check if user has MFA enabled
        // Implementation depends on how you store MFA settings
        return true; // For demo
    }

    private function verifyTotpCode($username, $code)
    {
        // Implement TOTP verification logic
        // Example: fetch user's secret, calculate expected code, compare
        return true; // For demo
    }

    private function generateAttemptId()
    {
        // Generate a unique attempt ID for server-side tracking
        return bin2hex(random_bytes(16));
    }
}
```

### Step 2: Create Challenge UI Component

For the challenge redirect, create an Angular component that:

1. Collects the challenge response from the user
2. Calls `Session::verify` with the response. The in-progress login is
   identified by the session cookie, so no token is required.

Example:

```typescript
// totp-challenge.component.ts
import { Component } from '@angular/core';
import { Router } from '@angular/router';
import { AuthService } from '~/app/shared/services/auth.service';

@Component({
  selector: 'omv-totp-challenge',
  template: `
    <mat-form-field>
      <mat-label>{{ 'Enter TOTP Code' | transloco }}</mat-label>
      <input matInput [(ngModel)]="totpCode" type="text" maxlength="6">
    </mat-form-field>
    <button mat-raised-button (click)="submit()">
      {{ 'Verify' | transloco }}
    </button>
  `
})
export class TotpChallengeComponent {
  totpCode = '';

  constructor(
    private authService: AuthService,
    private router: Router
  ) {}

  submit() {
    this.authService
      .verify({ code: this.totpCode })
      .subscribe({
        next: () => {
          this.router.navigate(['/dashboard']);
        },
        error: () => {
          // Error handled by interceptor
        }
      });
  }
}
```

### Step 3: Extend Router (optional)

Add a route for your challenge:

```typescript
// In your workbench routing
const routes: Routes = [
  {
    path: 'auth',
    children: [
      {
        path: 'totp',
        component: TotpChallengeComponent
      }
    ]
  }
];
```

---

## Security Considerations

### Pending Login State

- Stored server-side in the PHP session via `setPendingAuth()`, bound to the session cookie
- Never exposed to the client (no token in the response body)
- Expires after 5 minutes (fixed TTL from authenticate())
- Retrieved during verification via `getPendingAuth()`
- Discarded once the session is fully authenticated
- Contains:
  - Username (already authenticated)
  - Permissions (determined during authentication)
  - Full challenge object (including server-side-only fields)
- Contains no sensitive data (password is never stored)

### Challenge Responses

- Redirects MUST be relative URLs only (prevent open redirect)
- RPC params are validated with JSON schemas; challenge payloads/responses are plugin-defined
  and must be validated/sanitized by the plugin
- Challenge verification runs in admin context (trusted)
- Syslog all authentication attempts and failures
- **Never expose sensitive data to clients**: TOTP secrets, recovery codes, risk scores,
  attempt tracking IDs, or internal-only metadata should only be in server-side challenge fields
- The `Session::authenticate` method filters challenges using `array_pick_keys(['kind', 'redirecturl'])` 
  to ensure only safe fields reach the client

### Best Practices

1. **Never store passwords**: The pending login state is kept server-side and never includes passwords
2. **Separate client and server data**: Use server-side-only challenge fields for secrets, tracking IDs, 
   and internal metadata. Only `kind` and `redirecturl` are exposed to clients.
3. **Validate challenge responses**: Sanitize all input from `challengeresponse`
4. **Rate limit**: Consider implementing rate limits on verify attempts using server-side `attemptId` tracking
5. **Log everything**: Both successes and failures for audit trail (include `attemptId` for correlation)
6. **Use HTTPS**: Always deploy over HTTPS in production
7. **Mark hook methods as final**: Prevents accidental override of security-critical logic

---

## Testing

### Manual Test Flow

```bash
# Step 1: Authenticate (store the session cookie in a cookie jar).
curl -X POST http://localhost/rpc.php \
  -c cookies.txt \
  -H "Content-Type: application/json" \
  -d '{
    "service": "Session",
    "method": "authenticate",
    "params": {"username": "admin", "password": "admin"}
  }'

# Response if challenge required (only 'kind' and 'redirecturl' are exposed):
{
  "status": "challengeRequired",
  "username": "admin",
  "challenge": {
    "kind": "totp",
    "redirecturl": "/totp"
  }
}

# Step 2: Complete challenge. The session cookie identifies the pending login.
curl -X POST http://localhost/rpc.php \
  -b cookies.txt \
  -H "Content-Type: application/json" \
  -d '{
    "service": "Session",
    "method": "verify",
    "params": {
      "challengeresponse": {"code": "123456"}
    }
  }'

# Response on success:
{
  "authenticated": true,
  "username": "admin",
  "permissions": {...},
  "sessionid": "..."
}
```

---

## Migration from Old Login

`Session::login()` has been removed. Integrations should use
`Session::authenticate()` and, where a challenge is returned,
`Session::verify()`.

### For existing clients:
- Replace all calls to `Session::login()` with `Session::authenticate()`
- Check the `status` field in the response
- If `"authenticated"`, the session is established — proceed as before
- If `"challengeRequired"`, redirect to `challenge.redirecturl` and call
  `Session::verify()` after collecting the challenge response
- If a challenge kind-specific UI is used, route by `challenge.kind`

### For new clients:
- Use `Session::authenticate()` first
- Check response status
- If `"authenticated"`, proceed
- If `"challengeRequired"`, redirect to challenge URL
- After challenge, call `Session::verify()`
