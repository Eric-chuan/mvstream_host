head_index = []*25
for i in range (25):
    m3u8_file = open(f"/usr/local/nginx/www/LiveHLS/stream{i}.m3u8")
    lines = m3u8_file.readlines()
    line = lines[6]
    print(line)

