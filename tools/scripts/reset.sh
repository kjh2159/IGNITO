# product name
DEV="$(getprop ro.product.product.model)"
DEV="$(printf '%s' "$DEV" | tr -d '[:space:]')"
echo "Device: $DEV"

# turn-off screen
if [ "$DEV" = "Pixel9" ]; then
  # Pixel9
  
  # CPU
  su -c "echo 3105000 > /sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq;
  echo 700000 > /sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq;
  echo 2600000 > /sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq;
  echo 357000 > /sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq;
  echo 1950000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq;
  echo 820000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq;"

  # RAM
  su -c "echo 3744000 > /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/min_freq;
  echo 421000 > /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/max_freq;"
elif [ "$DEV" = "S24" ]; then
  # S24
  
  # CPU
  su -c "echo 3207000 > /sys/devices/system/cpu/cpufreq/policy9/scaling_max_freq;
  echo 672000 > /sys/devices/system/cpu/cpufreq/policy9/scaling_min_freq;
  echo 2900000 > /sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq;
  echo 672000 > /sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq;
  echo 2592000 > /sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq;
  echo 672000 > /sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq;
  echo 1959000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq;
  echo 400000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq;"

  # RAM
  su -c "echo 421000 > /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_min;
  echo 4206000 > /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_max;"
else
  echo "pass"
fi