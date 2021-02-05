#!/bin/bash
curl -v http://localhost:9876 1>/dev/null 2>&1
curl -v https://localhost:9876 1>/dev/null 2>&1 ; if [[ ${?} != 0 ]]; then echo "pass"; fi
curl -v -H "Content-Type: application/json" -d 'asdkfjlweoifj23{#$#$|}T#$% 3453' http://localhost:9876/poster 1>/dev/null 2>&1
curl -v -H "Content-Type: application/json" -d '{"value_one":123,"value_two":"Hello universe!"}' http://localhost:9876/poster 1>/dev/null 2>&1
curl -v -H "Content-Type: application/json" -d '{"value_one":123,"value_two":233423}' http://localhost:9876/poster 1>/dev/null 2>&1
curl -v -H "Content-Type: application/json" -d '{"value_one":123,"value_two":"Hello"}' http://localhost:9876/posterxxx 1>/dev/null 2>&1
curl -v -H "Content-Type: application/footype" -d '{"value_one":123,"value_two":"Hello"}' http://localhost:9876/posterxxx 1>/dev/null 2>&1
curl -v -H "Content-Type : application/json" -d '{"value_one":123,"value_two":"Hello"}' http://localhost:9876/posterxxx 1>/dev/null 2>&1
[[ ${?} == 0 ]] && echo "curl-tests passed"
