#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

unsigned double curr_window_size = 5; //Slow start, so begin with 5
unsigned int multiplicative_factor = 2; // The factor by which we decrease our window during a congestion event 
unsigned int additive_factor = 1 // The factor by which we increase our window during a congestion event 
uint64_t most_recent_window = -1 //Starting value for most_recent_window 

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = (unsigned int) curr_window_size;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Choose a scheme below */

  /*AIMD (similar to TCP Congestion avoidance) */
  if (sequence_number < most_recent_window) { //congestion has occured 
    curr_window_size /= multiplicative_factor; // multiplicative decrease
    cerr << "Congestion event detected! " << endl;
  } else {
    most_recent_window = sequence_number; // update most recent window sent
  }



  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */

  /*AIMD (similar to TCP Congestion avoidance) */
  curr_window_size += (additive_factor/curr_window_size); //additive increase, when acknowledgement is received

  if ( debug_ ) {
    cerr << "Current window size: " << (unsigned int) curr_window_size;
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
