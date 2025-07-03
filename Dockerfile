FROM ubuntu:22.04

WORKDIR /chain

# Install dependencies if needed
RUN apt-get update && apt-get install -y curl bash

COPY . .

# Make sure your script is executable
RUN chmod +x start_chain.sh

CMD ["./start_chain.sh"]
