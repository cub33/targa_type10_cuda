const rle_pkt_struct = {value: -1, repeats: 0}
let rle_pkt = Object.assign({}, rle_pkt_struct)
const raw_pkt_struct = {values: []}
let raw_pkt = Object.assign({}, raw_pkt_struct)
const packets = []
let different = true
const img = [
 [[1, 3, 4], [5, 3, 4], [1, 3, 4], [2, 3, 4], [3, 3, 4], [4, 3, 4]] ,
 [[5, 3, 4], [5, 3, 4], [5, 3, 4], [2, 3, 4], [2, 3, 4], [4, 3, 4]] ,
 [[5, 3, 4], [4, 3, 4], [2, 3, 4], [5, 3, 4], [4, 3, 4], [1, 3, 4]] ,
 [[5, 3, 4], [5, 3, 4], [2, 3, 4], [5, 3, 4], [5, 3, 4], [2, 3, 4]] ,
 [[5, 3, 4], [2, 3, 4], [2, 3, 4], [5, 3, 4], [4, 3, 4], [3, 3, 4]]
]

for (let y = 0; y < img[0].length; y++) {
  for (let x = 0; x < img.length; x++) {
    let value = img[x][y][0]
    let nextValue = -1
    let endLine = img[x+1] == undefined
    //let g = img[x][y][1];
    //let b = img[x][y][2];
    rle_pkt.value = value
    if (!endLine)
      nextValue = img[x+1][y][0]
    if (value == nextValue) {
      rle_pkt.repeats++
      different = false
      if (raw_pkt.values.length > 0) {
        packets.push(raw_pkt)
        raw_pkt = {values: []}
      }
    }
    else {
      if (different)
        raw_pkt.values.push(value)
      different = true
      let areRepeated = rle_pkt.repeats > 0
      if (areRepeated || (areRepeated && endLine)) {
        rle_pkt.repeats++
        packets.push(rle_pkt)
        rle_pkt = Object.assign({}, rle_pkt_struct)
      }
    }
  }
}
console.log(packets)
