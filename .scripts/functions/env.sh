source_env_file() {
    local env_file="$1"
    if [[ -f "$env_file" ]]; then
        log "Loading environment variables from $env_file"
        local export_lines=()
        while IFS= read -r line || [[ -n "$line" ]]; do
            [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]] && continue
            if [[ "$line" =~ ^[a-zA-Z_][a-zA-Z0-9_]*= ]]; then
                export "$line"
                export_lines+=("$line")
            else
                log_warn "Skipping invalid line in $env_file: $line"
            fi
        done < <(grep -v '^#' "$env_file" | grep -v '^$')
        # Expand variables that reference other variables
        for var in "${export_lines[@]}"; do
            var_name="${var%%=*}"
            eval "export $var_name=\"\${$var_name}\""
        done
    else
        error "$env_file not found!"
    fi
}

save_env_var() {
    local var_name="$1"
    local var_value="$2"
    local env_file="${3:-$ENV_FILE}"

    if [[ -z "$var_name" || -z "$var_value" ]]; then
        echo "Error: Missing variable name or value" >&2
        return 1
    fi

    mkdir -p "$(dirname "$env_file")"
    touch "$env_file"

    local escaped_value
    escaped_value=$(printf '%s' "$var_value" | sed 's/[$`\\]/\\&/g; s/[\r\n]/ /g')

    if grep -q "^$var_name=" "$env_file" 2>/dev/null; then
        # Entferne alte Zeile
        sed -i "/^$var_name=/d" "$env_file"
    fi

    if [[ -s "$env_file" && -n $(tail -c1 "$env_file") ]]; then
        echo "" >> "$env_file"
    fi

    echo "$var_name=$escaped_value" >> "$env_file"

    if [[ $? -ne 0 ]]; then
        echo "Error: Failed to write to $env_file" >&2
        return 1
    fi
}