cd bin\.web_zip

# Generate certificate for localhost if it doesn't exist
if (-not (Test-Path "localhost.pem")) {
    mkcert localhost
}

npx http-server -S -C localhost.pem -K localhost-key.pem -g -b -p 3000 `
    --header "Cross-Origin-Opener-Policy: same-origin" `
    --header "Cross-Origin-Embedder-Policy: require-corp"