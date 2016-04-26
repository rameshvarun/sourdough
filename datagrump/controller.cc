#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

double curr_window_size = 5; //Slow start, so begin with 5
unsigned int multiplicative_factor = 2; // The factor by which we decrease our window during a congestion event
unsigned int additive_factor = 1; // The factor by which we increase our window during a congestion event
unsigned int prop_delay_threshold = 155; // one-way propogation time threshold, for congestion event detection


/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = (unsigned int) curr_window_size;
  if (the_window_size < 1) the_window_size = 1;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms() << " window size is " <<
      the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{



  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

void Controller::on_timeout( void )
{
  /* Multiplicative decrease, when congestion is detected. */
  curr_window_size /= multiplicative_factor;
  if (curr_window_size < 1) curr_window_size = 1;
  // if (debug_) {
    cout << "Congestion event detected." << endl;
  // }
}

uint64_t prev_rtt = 0;

static uint64_t RTT_LOW = 0;
static uint64_t RTT_HIGH = 200;
static uint64_t MIN_RTT = 0;
static double alpha = 1.0;
static double beta = 1.0;
static double rtt_diff = 0;
static int negative_grad_counter = 0;

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
  uint64_t new_rtt = timestamp_ack_received - send_timestamp_acked;
  if (MIN_RTT == 0) {
    MIN_RTT = new_rtt;
    prev_rtt = new_rtt;
    cout << "Min RTT: " << MIN_RTT << endl;
    return;
  }

  double new_rtt_diff = (double)new_rtt - (double)prev_rtt;
  prev_rtt = new_rtt;
  rtt_diff = (1 - alpha)*rtt_diff + alpha*new_rtt_diff;
  double normalized_gradient = rtt_diff / MIN_RTT;
  if (new_rtt < RTT_LOW) {
    /* Additive increase of the window size. */
    curr_window_size += additive_factor / curr_window_size;
  } else if (new_rtt > RTT_HIGH) {
    /* Multiplicative decrease. */
    cout << "Mutliplicative decrease." << endl;
    curr_window_size /= multiplicative_factor;
  } else if (normalized_gradient <= 0) {
    if (negative_grad_counter > 5) {
      curr_window_size += 5 * additive_factor / curr_window_size;
    } else {
      curr_window_size += additive_factor / curr_window_size;
    }

    negative_grad_counter++;
  } else {
    //cout << normalized_gradient << endl;
    negative_grad_counter = 0;
    curr_window_size = curr_window_size - beta*normalized_gradient;
  }

  if (curr_window_size < 1) curr_window_size = 1;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
   << " received ack for datagram " << sequence_number_acked
   << " (send @ time " << send_timestamp_acked
   << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
   << endl;
  }
}

double Controller::send_rate( void ) {
  return 100.0;
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 200; /* timeout of one second */
}
