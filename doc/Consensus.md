# Consensus

## Data Formats


## Rules

#### Output Uniqueness

* Output commitments in the UTXO set must be unique at all times.
* A block cannot be included when it contains an output commitment which already exists in the UTXO set.
  * Block is still invalid even if it also contains an input with said commitment.
* A block cannot contain 2 outputs with the same commitment.
  * Block is still invalid even if it also contains an input with said commitment.
* A block *can* contain an input which spends an output created in the same block, provided the commitment was not already in the UTXO set.
  * To better support payment proofs, the outputs should still be added to the TXO MMR, but should be marked as spent.
---

#### Ordering

* Inputs in a block must be in ascending order by their raw commitment value.
* Outputs in a block must be in ascending order by their raw commitment value.
* Kernels in a block must be in ascending order by their raw hashed value.
* Signed owner messages must be in ascending order by their raw hashed value.
---

#### Signatures

#### Bulletproofs

#### Block Sums

#### Owner Sums