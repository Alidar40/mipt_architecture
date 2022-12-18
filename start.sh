sudo ./build/event_server --read=127.0.0.1 --write=127.0.0.1 --port=6033 --login=test --password=pzjqUkMnc7vfNHET --database=sql_test --init_db --preload data.json --cache_servers='127.0.0.1:10800' --queue=127.0.0.1:9092 --topic=event_server


