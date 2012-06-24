require 'test_helper'

class SequencesControllerTest < ActionController::TestCase
  setup do
    @sequence = sequences(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:sequences)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create sequence" do
    assert_difference('Sequence.count') do
      post :create, sequence: {  }
    end

    assert_redirected_to sequence_path(assigns(:sequence))
  end

  test "should show sequence" do
    get :show, id: @sequence
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @sequence
    assert_response :success
  end

  test "should update sequence" do
    put :update, id: @sequence, sequence: {  }
    assert_redirected_to sequence_path(assigns(:sequence))
  end

  test "should destroy sequence" do
    assert_difference('Sequence.count', -1) do
      delete :destroy, id: @sequence
    end

    assert_redirected_to sequences_path
  end
end
