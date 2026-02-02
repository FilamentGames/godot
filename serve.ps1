cd bin\.web_zip

# Generate certificate for localhost if it doesn't exist
if (-not (Test-Path "localhost+1.pem")) {
    mkcert localhost
}

# Generate serve.json with cross-origin isolation headers
@{
    headers = @(
        @{
            source = "**/*"
            headers = @(
                @{
                    key = "Cross-Origin-Opener-Policy"
                    value = "same-origin"
                }
                @{
                    key = "Cross-Origin-Embedder-Policy"
                    value = "require-corp"
                }
            )
        }
    )
} | ConvertTo-Json -Depth 10 | Out-File -FilePath "serve.json" -Encoding utf8

npx serve --ssl-cert localhost.pem --ssl-key localhost-key.pem