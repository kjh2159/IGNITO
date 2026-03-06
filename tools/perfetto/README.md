# Perfetto analysis

## Before Reading

The collection of perfetto data should be conducted through [here](https://github.com/kjh2159/mllm/tree/main/perfetto)

## Example

A simple example of perfetto trace for reproduction is provided like the following.

```shell
# remove exisiting cache data
adb shell su -c 'rm -f /data/local/tmp/traces.perfetto-trace'
adb shell su -c 'rm -f /data/local/tmp/trace_config.pbtxt'

# set the trace configuration and perfetto run
adb push perfetto/trace_config.pbtxt /data/local/tmp/
adb shell su -c 'perfetto --txt -c /data/local/tmp/trace_config.pbtxt -o /data/local/tmp/traces.perfetto-trace'

# extract the perfetto output
adb shell su -c 'chmod 755 /data/local/tmp/traces.perfetto-trace'
adb pull /data/local/tmp/traces.perfetto-trace ./ 

```

--- 

## Reference

- https://perfetto.dev/docs/data-sources/battery-counters#odpm
- https://source.chromium.org/chromium/chromium/src/+/main:third_party/perfetto//protos/perfetto/trace/ftrace/regulator.proto
