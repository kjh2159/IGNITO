# product name
DEV="$(getprop ro.product.product.model)"
DEV="$(printf '%s' "$DEV" | tr -d '[:space:]')"
echo "Device: $DEV"

# turn-off screen
if [ "$DEV" = "Pixel9" ]; then
  # Pixel9
  echo 0 > /sys/class/backlight/panel0-backlight/brightness
else
  # S24
  echo 0 > /sys/class/backlight/panel/brightness
  DEV="S24"
fi

sleep 3 # stabilize

# silver core control
# su -c "echo 0 > /sys/devices/system/cpu/cpu1/online"
# su -c "echo 0 > /sys/devices/system/cpu/cpu2/online"
# su -c "echo 0 > /sys/devices/system/cpu/cpu3/online"

./bin-arm/stream_llama3 \
  -m models/llama-3.2-1b-q4k-inst.mllm \
  -v vocab/llama3_tokenizer.model \
  -b 1b \
  -t 4 \
  -l 1024 \
  -i 1 \
  -s 1 \
  -L 20 \
  -I dataset/hotpot_qa.csv \
  -O output/ \
  -S 0 \
  -D "$DEV" \
  --strict 0 \
  --cpu-p $1 \
  --ram-p $2 \
  --cpu-d $3 \
  --ram-d $4 \
  --phase-pause 0 \
  --token-pause 0 \
  --layer-pause 0 \
  --query-interval 0

# [pause-unit] = ms
# [interval-unit] = s

# silver core reset
# su -c "echo 1 > /sys/devices/system/cpu/cpu1/online"
# su -c "echo 1 > /sys/devices/system/cpu/cpu2/online"
# su -c "echo 1 > /sys/devices/system/cpu/cpu3/online"

# turn-on screen
# turn-off screen
if [ "$DEV" = "Pixel9" ]; then
  # Pixel9
  echo 1023 > /sys/class/backlight/panel0-backlight/brightness
else
  # S24
  echo 1023 > /sys/class/backlight/panel/brightness
fi
