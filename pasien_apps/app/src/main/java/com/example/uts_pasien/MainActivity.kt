package com.example.uts_pasien

import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import com.google.firebase.database.*

class MainActivity : AppCompatActivity() {

    private val database = FirebaseDatabase.getInstance()
    private val myRef = database.getReference("UTS")

    private lateinit var button01: Button
    private lateinit var button02: Button

    private var countButton01 = 0
    private var countButton02 = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        button01 = findViewById(R.id.button01)
        button02 = findViewById(R.id.button02)

        button01.setOnClickListener {
            countButton01++
            myRef.child("Pasien01").child("Count").setValue(countButton01)

            myRef.child("Pasien01/Status").addListenerForSingleValueEvent(object : ValueEventListener {
                override fun onDataChange(dataSnapshot: DataSnapshot) {
                    val status = dataSnapshot.value as Long
                    if (status == 0L) {
                        myRef.child("Pasien01").child("Status").setValue(1)
                        myRef.child("Pasien01").child("Pesan").setValue("Pasien 01 meminta bantuan")
                        myRef.child("Pasien02").child("Status").setValue(0)
                        myRef.child("Pasien02").child("Pesan").setValue("")
                    }
                }

                override fun onCancelled(error: DatabaseError) {
                    // Handle error
                }
            })
        }

        button02.setOnClickListener {
            countButton02++
            myRef.child("Pasien02").child("Count").setValue(countButton02)

            myRef.child("Pasien02/Status").addListenerForSingleValueEvent(object : ValueEventListener {
                override fun onDataChange(dataSnapshot: DataSnapshot) {
                    val status = dataSnapshot.value as Long
                    if (status == 0L) {
                        myRef.child("Pasien02").child("Status").setValue(1)
                        myRef.child("Pasien02").child("Pesan").setValue("Pasien 02 meminta bantuan")
                        myRef.child("Pasien01").child("Status").setValue(0)
                        myRef.child("Pasien01").child("Pesan").setValue("")
                    }
                }

                override fun onCancelled(error: DatabaseError) {
                    // Handle error
                }
            })
        }
    }
}
